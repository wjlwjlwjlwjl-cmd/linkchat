import { useCallback, useEffect, useRef, useState } from "react";
import type { ReactNode } from "react";
import { createPortal } from "react-dom";
import {
  ChevronLeft,
  ChevronRight,
  Download,
  RotateCw,
  X,
  ZoomIn,
  ZoomOut,
} from "lucide-react";
import { getSingleFile } from "@/api/file";
import { useMessageStore } from "@/stores/message";
import { useAvatarUrl } from "@/hooks/useAvatarUrl";
import { imageMimeType, isImageFileName } from "@/lib/file";
import { cn } from "@/components/ui/cn";

interface ImageViewerDialogProps {
  sessionId: string;
  startMessageId: string;
  onClose: () => void;
}

interface ImageItem {
  messageId: string;
  fileName: string;
  fileId?: string;
  fileContent: string;
}

const MIN_SCALE = 0.5;
const MAX_SCALE = 3;
const SCALE_STEP = 0.25;

// 单张图片：优先本地 base64，历史消息用 file_id 换预签名地址
function ViewerImage({ item }: { item: ImageItem }) {
  const remoteUrl = useAvatarUrl(item.fileContent ? undefined : item.fileId);
  const src = item.fileContent
    ? `data:${imageMimeType(item.fileName)};base64,${item.fileContent}`
    : remoteUrl;

  if (!src) {
    return <div className="h-40 w-40 rounded-base bg-input" />;
  }
  return (
    <img
      src={src}
      alt={item.fileName}
      className="max-h-[calc(100vh-160px)] max-w-[calc(100vw-160px)] select-none object-contain"
      draggable={false}
    />
  );
}

// 工具条按钮：不可用时置灰
function ToolButton({
  label,
  disabled,
  onClick,
  children,
}: {
  label: string;
  disabled?: boolean;
  onClick: () => void;
  children: ReactNode;
}) {
  return (
    <button
      type="button"
      aria-label={label}
      title={label}
      disabled={disabled}
      onClick={onClick}
      className={cn(
        "hv-icon rounded-base p-1.5 text-secondary",
        disabled &&
          "pointer-events-none text-tertiary opacity-45 disabled:text-tertiary",
      )}
    >
      {children}
    </button>
  );
}

// 图片浏览弹窗：上一张/下一张、缩放、旋转、下载
export function ImageViewerDialog({
  sessionId,
  startMessageId,
  onClose,
}: ImageViewerDialogProps) {
  // 打开时收集当前会话内全部图片消息，作为浏览列表
  const [items] = useState<ImageItem[]>(() => {
    const messages = useMessageStore.getState().messages[sessionId] ?? [];
    return messages
      .filter(
        (m) =>
          m.message.fileMessage !== undefined &&
          isImageFileName(m.message.fileMessage.fileName),
      )
      .map((m) => {
        const file = m.message.fileMessage;
        return {
          messageId: m.messageId,
          fileName: file?.fileName ?? "图片",
          fileId: file?.fileId,
          fileContent: file?.fileContent ?? "",
        };
      });
  });

  const [index, setIndex] = useState(() => {
    const found = items.findIndex((i) => i.messageId === startMessageId);
    return found >= 0 ? found : 0;
  });
  const [scale, setScale] = useState(1);
  const [rotation, setRotation] = useState(0);
  const [toast, setToast] = useState("");
  const toastTimer = useRef<number | undefined>(undefined);

  const count = items.length;
  const current = items[index];

  const go = useCallback(
    (delta: number) => {
      setIndex((prev) => {
        const next = Math.min(count - 1, Math.max(0, prev + delta));
        if (next !== prev) {
          setScale(1);
          setRotation(0);
        }
        return next;
      });
    },
    [count],
  );

  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") {
        onClose();
      } else if (e.key === "ArrowLeft") {
        go(-1);
      } else if (e.key === "ArrowRight") {
        go(1);
      }
    };
    window.addEventListener("keydown", onKeyDown);
    return () => window.removeEventListener("keydown", onKeyDown);
  }, [go, onClose]);

  useEffect(() => {
    return () => window.clearTimeout(toastTimer.current);
  }, []);

  const showToast = (text: string) => {
    setToast(text);
    window.clearTimeout(toastTimer.current);
    toastTimer.current = window.setTimeout(() => setToast(""), 2000);
  };

  const zoom = (delta: number) => {
    setScale((prev) => {
      const next = Math.min(MAX_SCALE, Math.max(MIN_SCALE, prev + delta));
      return Math.round(next * 100) / 100;
    });
  };

  const handleDownload = async () => {
    if (!current) return;
    try {
      let url = "";
      if (current.fileContent) {
        const binary = atob(current.fileContent);
        const bytes = new Uint8Array(binary.length);
        for (let i = 0; i < binary.length; i += 1) {
          bytes[i] = binary.charCodeAt(i);
        }
        url = URL.createObjectURL(new Blob([bytes]));
      } else if (current.fileId) {
        const rsp = await getSingleFile(current.fileId);
        url = rsp.success === true ? (rsp.fileData?.presignedUrl ?? "") : "";
      }
      if (!url) {
        showToast("下载失败");
        return;
      }
      const link = document.createElement("a");
      link.href = url;
      link.download = current.fileName;
      link.rel = "noopener";
      document.body.appendChild(link);
      link.click();
      link.remove();
      showToast("下载成功");
    } catch {
      showToast("下载失败");
    }
  };

  if (!current) return null;

  return createPortal(
    <div
      role="dialog"
      aria-modal="true"
      aria-label="图片浏览"
      className="fixed inset-0 z-50 bg-rail"
      onMouseDown={(e) => {
        if (e.target === e.currentTarget) onClose();
      }}
    >
      {/* 顶部工具栏 */}
      <div className="absolute inset-x-0 top-4 z-10 flex justify-center">
        <div className="flex items-center gap-1 rounded-pill border border-line bg-panel px-2 py-1 shadow-popup">
          <ToolButton
            label="上一张"
            disabled={index <= 0}
            onClick={() => go(-1)}
          >
            <ChevronLeft size={20} strokeWidth={1.5} />
          </ToolButton>
          <ToolButton
            label="下一张"
            disabled={index >= count - 1}
            onClick={() => go(1)}
          >
            <ChevronRight size={20} strokeWidth={1.5} />
          </ToolButton>
          <span className="mx-1 h-5 w-px bg-line" />
          <ToolButton
            label="缩小"
            disabled={scale <= MIN_SCALE}
            onClick={() => zoom(-SCALE_STEP)}
          >
            <ZoomOut size={20} strokeWidth={1.5} />
          </ToolButton>
          <ToolButton
            label="放大"
            disabled={scale >= MAX_SCALE}
            onClick={() => zoom(SCALE_STEP)}
          >
            <ZoomIn size={20} strokeWidth={1.5} />
          </ToolButton>
          <ToolButton label="旋转" onClick={() => setRotation((r) => r + 90)}>
            <RotateCw size={20} strokeWidth={1.5} />
          </ToolButton>
          <ToolButton label="下载" onClick={() => void handleDownload()}>
            <Download size={20} strokeWidth={1.5} />
          </ToolButton>
        </div>
      </div>

      <button
        type="button"
        aria-label="关闭"
        onClick={onClose}
        className="hv-icon absolute right-4 top-4 z-10 rounded-base border border-line bg-panel p-1.5 text-secondary shadow-popup"
      >
        <X size={20} strokeWidth={1.5} />
      </button>

      {/* 图片区域：点击空白处关闭 */}
      <div
        className="flex h-full items-center justify-center px-20 py-24"
        onMouseDown={(e) => {
          if (e.target === e.currentTarget) onClose();
        }}
      >
        <div
          className="transition-transform duration-150 ease-out"
          style={{ transform: `scale(${scale}) rotate(${rotation}deg)` }}
        >
          <ViewerImage item={current} />
        </div>
      </div>

      {/* 居中提示 */}
      {toast && (
        <div className="pointer-events-none absolute left-1/2 top-1/2 z-20 -translate-x-1/2 -translate-y-1/2 rounded-base border border-line bg-panel px-4 py-2 text-sm text-primary shadow-popup">
          {toast}
        </div>
      )}
    </div>,
    document.body,
  );
}
