import { useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";

interface AvatarCropDialogProps {
  file: File;
  onConfirm: (blob: Blob) => Promise<void>;
  onCancel: () => void;
}

const VIEW = 240; // 预览视口边长（px，Tailwind h-60 / w-60）
const OUTPUT = 256; // 裁剪输出边长（px）
const MAX_ZOOM = 4;
const FOCUSABLE =
  'button:not([disabled]), input:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

// 按 cover 铺满正方形容器，并叠加缩放与平移（offset 以 VIEW 像素为基准）
function paint(
  ctx: CanvasRenderingContext2D,
  img: HTMLImageElement,
  zoom: number,
  offset: { x: number; y: number },
  size: number,
): void {
  const k = size / VIEW;
  const base = Math.max(size / img.naturalWidth, size / img.naturalHeight);
  const w = img.naturalWidth * base * zoom;
  const h = img.naturalHeight * base * zoom;
  ctx.drawImage(
    img,
    (size - w) / 2 + offset.x * k,
    (size - h) / 2 + offset.y * k,
    w,
    h,
  );
}

// 限制平移范围，保证图片始终盖满视口
function clampOffset(
  img: HTMLImageElement,
  zoom: number,
  offset: { x: number; y: number },
): { x: number; y: number } {
  const base = Math.max(VIEW / img.naturalWidth, VIEW / img.naturalHeight);
  const maxX = Math.max(0, (img.naturalWidth * base * zoom - VIEW) / 2);
  const maxY = Math.max(0, (img.naturalHeight * base * zoom - VIEW) / 2);
  return {
    x: Math.min(maxX, Math.max(-maxX, offset.x)),
    y: Math.min(maxY, Math.max(-maxY, offset.y)),
  };
}

// 头像裁剪弹窗：正方形视口 + 拖拽平移 + 滑块缩放，确认后输出 PNG
export function AvatarCropDialog({
  file,
  onConfirm,
  onCancel,
}: AvatarCropDialogProps) {
  const [img, setImg] = useState<HTMLImageElement | null>(null);
  const [zoom, setZoom] = useState(1);
  const [offset, setOffset] = useState({ x: 0, y: 0 });
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState("");
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const dialogRef = useRef<HTMLDivElement>(null);
  const dragRef = useRef<{
    x: number;
    y: number;
    ox: number;
    oy: number;
  } | null>(null);

  // 解码本地文件，失败时就地提示
  useEffect(() => {
    const url = URL.createObjectURL(file);
    const image = new Image();
    image.onload = () => setImg(image);
    image.onerror = () => setError("图片加载失败，请换一张");
    image.src = url;
    return () => {
      image.onload = null;
      URL.revokeObjectURL(url);
    };
  }, [file]);

  // 预览重绘（按设备像素比保持清晰）
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !img) return;
    const dpr = window.devicePixelRatio || 1;
    canvas.width = VIEW * dpr;
    canvas.height = VIEW * dpr;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, VIEW, VIEW);
    paint(ctx, img, zoom, clampOffset(img, zoom, offset), VIEW);
  }, [img, zoom, offset]);

  // 滚轮缩放（passive: false 才能阻止页面滚动）
  useEffect(() => {
    const el = canvasRef.current;
    if (!el || !img) return;
    const onWheel = (e: WheelEvent) => {
      e.preventDefault();
      const next = Math.min(MAX_ZOOM, Math.max(1, zoom - e.deltaY * 0.002));
      setZoom(next);
      setOffset((o) => clampOffset(img, next, o));
    };
    el.addEventListener("wheel", onWheel, { passive: false });
    return () => el.removeEventListener("wheel", onWheel);
  }, [img, zoom]);

  // 打开时聚焦弹窗，Esc 关闭
  useEffect(() => {
    dialogRef.current?.focus();
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Tab") {
        const container = dialogRef.current;
        if (!container) return;
        const items = container.querySelectorAll<HTMLElement>(FOCUSABLE);
        if (items.length === 0) return;
        const first = items[0];
        const last = items[items.length - 1];
        if (
          e.shiftKey &&
          (document.activeElement === first ||
            document.activeElement === container)
        ) {
          e.preventDefault();
          last.focus();
        } else if (!e.shiftKey && document.activeElement === last) {
          e.preventDefault();
          first.focus();
        }
        return;
      }
      if (e.key === "Escape" && !saving) onCancel();
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [onCancel, saving]);

  const changeZoom = (next: number) => {
    const value = Math.min(MAX_ZOOM, Math.max(1, next));
    setZoom(value);
    if (img) setOffset((o) => clampOffset(img, value, o));
  };

  const handlePointerDown = (e: React.PointerEvent<HTMLCanvasElement>) => {
    if (!img || saving) return;
    e.currentTarget.setPointerCapture(e.pointerId);
    dragRef.current = {
      x: e.clientX,
      y: e.clientY,
      ox: offset.x,
      oy: offset.y,
    };
  };

  const handlePointerMove = (e: React.PointerEvent<HTMLCanvasElement>) => {
    const drag = dragRef.current;
    if (!drag || !img) return;
    setOffset(
      clampOffset(img, zoom, {
        x: drag.ox + e.clientX - drag.x,
        y: drag.oy + e.clientY - drag.y,
      }),
    );
  };

  const handlePointerUp = (e: React.PointerEvent<HTMLCanvasElement>) => {
    if (e.currentTarget.hasPointerCapture(e.pointerId)) {
      e.currentTarget.releasePointerCapture(e.pointerId);
    }
    dragRef.current = null;
  };

  const handleSave = async () => {
    if (!img || saving) return;
    setSaving(true);
    setError("");
    try {
      const out = document.createElement("canvas");
      out.width = OUTPUT;
      out.height = OUTPUT;
      const ctx = out.getContext("2d");
      if (!ctx) throw new Error("无法生成头像");
      paint(ctx, img, zoom, clampOffset(img, zoom, offset), OUTPUT);
      const blob = await new Promise<Blob | null>((resolve) =>
        out.toBlob(resolve, "image/png"),
      );
      if (!blob) throw new Error("无法生成头像");
      await onConfirm(blob);
      onCancel();
    } catch (err) {
      setError(err instanceof Error ? err.message : "保存失败");
    } finally {
      setSaving(false);
    }
  };

  return createPortal(
    <div
      className="fixed inset-0 z-50 flex items-center justify-center"
      style={{ backgroundColor: "var(--overlay-bg)" }}
      onMouseDown={(e) => {
        if (e.target === e.currentTarget && !saving) onCancel();
      }}
    >
      <div
        ref={dialogRef}
        role="dialog"
        aria-modal="true"
        aria-label="裁剪头像"
        tabIndex={-1}
        className="w-[360px] rounded-lg border border-line bg-panel p-5 shadow-popup focus:outline-none"
      >
        <h2 className="mb-4 text-base font-medium text-primary">裁剪头像</h2>

        <div className="relative mx-auto h-60 w-60 overflow-hidden rounded-base border border-line bg-rail">
          <canvas
            ref={canvasRef}
            className="h-full w-full touch-none"
            style={{ cursor: img ? "grab" : "default" }}
            onPointerDown={handlePointerDown}
            onPointerMove={handlePointerMove}
            onPointerUp={handlePointerUp}
            onPointerCancel={handlePointerUp}
          />
          {!img && !error && (
            <span className="pointer-events-none absolute inset-0 flex items-center justify-center text-sm text-inverse">
              加载中…
            </span>
          )}
        </div>

        <div className="mt-4 flex items-center gap-3">
          <span className="shrink-0 text-sm text-secondary">缩放</span>
          <input
            type="range"
            min={1}
            max={MAX_ZOOM}
            step={0.01}
            value={zoom}
            aria-label="缩放比例"
            disabled={!img}
            onChange={(e) => changeZoom(Number(e.target.value))}
            className="min-w-0 flex-1"
          />
          <span className="w-10 shrink-0 text-right text-sm text-secondary tabular-nums">
            {zoom.toFixed(1)}×
          </span>
        </div>

        {error && (
          <p
            role="alert"
            className="mt-3 text-sm"
            style={{ color: "var(--color-danger)" }}
          >
            {error}
          </p>
        )}

        <div className="mt-5 flex justify-end gap-2">
          <button
            type="button"
            onClick={onCancel}
            disabled={saving}
            className="hv-icon h-8 rounded-base border border-line px-4 text-sm text-primary disabled:opacity-50"
          >
            取消
          </button>
          <button
            type="button"
            onClick={() => void handleSave()}
            disabled={!img || saving}
            className="h-8 rounded-base px-4 text-sm disabled:cursor-not-allowed disabled:opacity-50"
            style={{
              background: "var(--color-brand)",
              color: "var(--text-on-brand)",
            }}
          >
            {saving ? "保存中…" : "保存"}
          </button>
        </div>
      </div>
    </div>,
    document.body,
  );
}
