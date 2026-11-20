import { useEffect, useRef, useState } from "react";
import { Pencil } from "lucide-react";
import { Avatar } from "@/components/Avatar";
import { AvatarCropDialog } from "@/components/AvatarCropDialog";
import {
  getProfile,
  setUserAvatar,
  setUserDescription,
  setUserNickname,
} from "@/api/user";
import { useAuthStore } from "@/stores/auth";
import type { UserInfo } from "@/types";

interface ProfilePopoverProps {
  onClose: () => void;
  returnFocusRef: React.RefObject<HTMLButtonElement>;
}

const MAX_AVATAR_SIZE = 2 * 1024 * 1024; // 头像原图上限 2MB
const AVATAR_TYPES = ["image/png", "image/jpeg", "image/webp"];
const NICKNAME_MAX = 20;
const DESCRIPTION_MAX = 50;
const FOCUSABLE =
  'button:not([disabled]), input:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

// 个人资料卡：大头像（可点击更换）+ 昵称 / 签名 + 行内编辑
export function ProfilePopover({
  onClose,
  returnFocusRef,
}: ProfilePopoverProps) {
  const profile = useAuthStore((s) => s.profile);
  const setProfile = useAuthStore((s) => s.setProfile);
  const [editing, setEditing] = useState<"nickname" | "description" | null>(
    null,
  );
  const [draft, setDraft] = useState("");
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState("");
  const [cropFile, setCropFile] = useState<File | null>(null);
  const rootRef = useRef<HTMLDivElement>(null);
  const fileRef = useRef<HTMLInputElement>(null);

  // 打开时焦点进入资料卡
  useEffect(() => {
    rootRef.current?.focus();
  }, []);

  // Esc 逐级关闭（编辑 → 资料卡），点击外部关闭；裁剪弹窗打开时交给弹窗处理
  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
      if (cropFile) return;
      // Tab 在资料卡内循环（style.md §3.3 焦点陷阱）
      if (e.key === "Tab") {
        const container = rootRef.current;
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
      if (e.key !== "Escape" || saving) return;
      if (editing) {
        setEditing(null);
        setError("");
        return;
      }
      onClose();
      returnFocusRef.current?.focus();
    };
    const onPointerDown = (e: MouseEvent) => {
      if (cropFile) return;
      const target = e.target as Node;
      if (
        rootRef.current?.contains(target) ||
        returnFocusRef.current?.contains(target)
      ) {
        return;
      }
      onClose();
    };
    window.addEventListener("keydown", onKeyDown);
    window.addEventListener("mousedown", onPointerDown);
    return () => {
      window.removeEventListener("keydown", onKeyDown);
      window.removeEventListener("mousedown", onPointerDown);
    };
  }, [cropFile, editing, onClose, returnFocusRef, saving]);

  // 刷新当前用户资料；失败时用本次修改值兜底，避免界面不更新
  const refreshProfile = async (patch: Partial<UserInfo>): Promise<void> => {
    try {
      const rsp = await getProfile();
      if (rsp.success === true && rsp.userInfo) {
        setProfile(rsp.userInfo);
        return;
      }
    } catch {
      // 刷新失败走本地兜底
    }
    if (profile) setProfile({ ...profile, ...patch });
  };

  // 上传裁剪后的头像，完成后重新拉取资料拿到新 file_id
  const uploadAvatar = async (blob: Blob): Promise<void> => {
    const rsp = await setUserAvatar(blob);
    if (rsp.success !== true) throw new Error(rsp.errmsg || "头像修改失败");
    const info = await getProfile();
    if (info.success === true && info.userInfo) {
      setProfile(info.userInfo);
      return;
    }
    throw new Error("头像已更新，但资料刷新失败，请刷新页面");
  };

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    e.target.value = "";
    if (!file) return;
    if (!AVATAR_TYPES.includes(file.type)) {
      setError("仅支持 PNG / JPEG / WebP 图片");
      return;
    }
    if (file.size > MAX_AVATAR_SIZE) {
      setError("图片不能超过 2MB");
      return;
    }
    setError("");
    setCropFile(file);
  };

  const startEdit = (field: "nickname" | "description") => {
    setEditing(field);
    setDraft(
      field === "nickname"
        ? (profile?.nickname ?? "")
        : (profile?.description ?? ""),
    );
    setError("");
  };

  const cancelEdit = () => {
    setEditing(null);
    setError("");
  };

  const saveEdit = async () => {
    if (!editing || saving) return;
    const value = draft.trim();
    if (!value) {
      setError(editing === "nickname" ? "昵称不能为空" : "签名不能为空");
      return;
    }
    setSaving(true);
    setError("");
    try {
      const rsp =
        editing === "nickname"
          ? await setUserNickname(value)
          : await setUserDescription(value);
      if (rsp.success !== true) throw new Error(rsp.errmsg || "修改失败");
      await refreshProfile(
        editing === "nickname" ? { nickname: value } : { description: value },
      );
      setEditing(null);
    } catch (err) {
      setError(err instanceof Error ? err.message : "修改失败");
    } finally {
      setSaving(false);
    }
  };

  // Enter 保存，Shift+Enter 换行，IME 组合期间不提交
  const handleKeyDown = (
    e: React.KeyboardEvent<HTMLInputElement | HTMLTextAreaElement>,
  ) => {
    if (e.key !== "Enter" || e.shiftKey) return;
    if (e.nativeEvent.isComposing) return;
    e.preventDefault();
    void saveEdit();
  };

  const nickname = profile?.nickname ?? "我";
  const signature = profile?.description || "暂无签名";
  const email = profile?.email || "未绑定";

  const editActions = (
    <div className="mt-2 flex justify-end gap-2">
      <button
        type="button"
        onClick={cancelEdit}
        disabled={saving}
        className="hv-icon h-7 rounded-base px-3 text-sm text-secondary disabled:opacity-50"
      >
        取消
      </button>
      <button
        type="button"
        onClick={() => void saveEdit()}
        disabled={saving || draft.trim().length === 0}
        className="h-7 rounded-base px-3 text-sm disabled:cursor-not-allowed disabled:opacity-50"
        style={{
          background: "var(--color-brand)",
          color: "var(--text-on-brand)",
        }}
      >
        {saving ? "保存中…" : "保存"}
      </button>
    </div>
  );

  return (
    <div
      ref={rootRef}
      role="dialog"
      aria-label="个人资料"
      tabIndex={-1}
      className="w-80 rounded-lg border border-line bg-panel p-4 shadow-popup focus:outline-none"
    >
      <div className="flex items-start gap-3">
        <div className="relative shrink-0">
          <button
            type="button"
            aria-label="更换头像"
            title="点击更换头像"
            onClick={() => fileRef.current?.click()}
            className="block rounded-base"
          >
            <Avatar fileId={profile?.avatar} name={nickname} size={64} />
          </button>
          <span
            aria-hidden="true"
            className="pointer-events-none absolute -bottom-1 -right-1 flex h-5 w-5 items-center justify-center rounded-full border border-line bg-panel text-secondary"
          >
            <Pencil size={12} strokeWidth={1.5} />
          </span>
          <input
            ref={fileRef}
            type="file"
            accept="image/png,image/jpeg,image/webp"
            className="hidden"
            onChange={handleFileChange}
          />
        </div>
        <div className="min-w-0 flex-1 pt-0.5">
          <p className="truncate text-base font-medium text-primary">
            {nickname}
          </p>
          <p className="mt-1 truncate text-sm text-secondary">邮箱：{email}</p>
          <p className="mt-1 line-clamp-2 break-words text-sm text-secondary">
            {signature}
          </p>
        </div>
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

      {editing === "nickname" ? (
        <div className="mt-4">
          <input
            autoFocus
            value={draft}
            maxLength={NICKNAME_MAX}
            aria-label="昵称"
            placeholder="请输入昵称"
            onChange={(e) => setDraft(e.target.value)}
            onKeyDown={handleKeyDown}
            className="h-8 w-full rounded-base border border-line bg-input px-2 text-sm text-primary placeholder:text-tertiary"
          />
          {editActions}
        </div>
      ) : editing === "description" ? (
        <div className="mt-4">
          <textarea
            autoFocus
            rows={2}
            value={draft}
            maxLength={DESCRIPTION_MAX}
            aria-label="签名"
            placeholder="请输入签名"
            onChange={(e) => setDraft(e.target.value)}
            onKeyDown={handleKeyDown}
            className="w-full resize-none rounded-base border border-line bg-input px-2 py-1.5 text-sm text-primary placeholder:text-tertiary"
          />
          {editActions}
        </div>
      ) : (
        <div className="mt-4 flex gap-2">
          <button
            type="button"
            onClick={() => startEdit("nickname")}
            className="hv-icon h-7 flex-1 rounded-base border border-line text-sm text-primary"
          >
            更改昵称
          </button>
          <button
            type="button"
            onClick={() => startEdit("description")}
            className="hv-icon h-7 flex-1 rounded-base border border-line text-sm text-primary"
          >
            更改签名
          </button>
        </div>
      )}

      {cropFile && (
        <AvatarCropDialog
          file={cropFile}
          onConfirm={uploadAvatar}
          onCancel={() => setCropFile(null)}
        />
      )}
    </div>
  );
}
