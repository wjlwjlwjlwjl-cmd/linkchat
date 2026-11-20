import { useRef, useState } from "react";
import { Smile, Image, Paperclip, Loader2, X } from "lucide-react";
import { EmojiPicker } from "@/components/EmojiPicker";
import type { EmojiItem } from "@/components/emojiData";
import { cn } from "@/components/ui/cn";
import { useSendMessage } from "@/hooks/useSendMessage";
import { imageMimeType, isImageFileName } from "@/lib/file";

interface InputAreaProps {
  sessionId: string;
}

// 待发送图片（选择后先暂存在输入区，点击发送或回车才真正发送）
interface PendingImage {
  id: string;
  fileName: string;
  fileSize: number;
  content: string;
}

// 前端单张图片上限 10MB（后端消息上限 20MB），单次最多 9 张
const MAX_IMAGE_BYTES = 10 * 1024 * 1024;
const MAX_IMAGE_COUNT = 9;
// 文件上限 15MB（后端消息上限 20MB）
const MAX_FILE_BYTES = 15 * 1024 * 1024;

// 读取为不带 data URL 前缀的 base64
function readAsBase64(file: File): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => {
      const result = typeof reader.result === "string" ? reader.result : "";
      resolve(result.slice(result.indexOf(",") + 1));
    };
    reader.onerror = () => reject(reader.error);
    reader.readAsDataURL(file);
  });
}

// 输入区（style.md §2.5 / §4.8）：Enter 发送，Shift+Enter 换行，
// IME 组合期间 Enter 不发送；工具栏位于输入框下方
export function InputArea({ sessionId }: InputAreaProps) {
  const { send, sendFile, sendImages } = useSendMessage(sessionId);
  const [text, setText] = useState("");
  const [emojiOpen, setEmojiOpen] = useState(false);
  const [uploading, setUploading] = useState(false);
  const [images, setImages] = useState<PendingImage[]>([]);
  const [error, setError] = useState("");
  const composingRef = useRef(false);
  const textareaRef = useRef<HTMLTextAreaElement>(null);
  const emojiBtnRef = useRef<HTMLButtonElement>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);
  const fileDocInputRef = useRef<HTMLInputElement>(null);
  const canSend = text.trim().length > 0 || images.length > 0;

  // 发送：先发暂存图片，再发文字；全部发起后清空对应内容
  const submit = async () => {
    const content = text.trim();
    if ((!content && images.length === 0) || uploading) return;
    setError("");
    setUploading(true);
    try {
      if (images.length > 0) {
        await sendImages(images);
        setImages([]);
      }
      if (content) {
        const ok = await send(content);
        if (ok) setText("");
      }
    } finally {
      setUploading(false);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key !== "Enter" || e.shiftKey) return;
    // 中文输入法组合期间不发送
    if (composingRef.current || e.nativeEvent.isComposing) return;
    e.preventDefault();
    void submit();
  };

  // 在光标处插入表情，并恢复焦点与光标位置
  const insertEmoji = (emoji: EmojiItem) => {
    const node = textareaRef.current;
    const start = node?.selectionStart ?? text.length;
    const end = node?.selectionEnd ?? text.length;
    const caret = start + emoji.char.length;
    setText(text.slice(0, start) + emoji.char + text.slice(end));
    requestAnimationFrame(() => {
      const el = textareaRef.current;
      if (!el) return;
      el.focus();
      el.setSelectionRange(caret, caret);
    });
  };

  // 选中表情后插入，并关闭表情面板
  const pickEmoji = (emoji: EmojiItem) => {
    insertEmoji(emoji);
    setEmojiOpen(false);
  };

  // 选择图片：校验类型与大小后暂存到输入区（不直接发送）
  const handleImageChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const files = Array.from(e.target.files ?? []);
    e.target.value = "";
    if (files.length === 0) return;

    setError("");
    setEmojiOpen(false);

    const remain = MAX_IMAGE_COUNT - images.length;
    if (remain <= 0) {
      setError(`最多只能发送 ${MAX_IMAGE_COUNT} 张图片`);
      return;
    }
    if (files.length > remain) {
      setError(
        `最多只能发送 ${MAX_IMAGE_COUNT} 张图片，本次仅添加 ${remain} 张`,
      );
    }
    const accepted = files.slice(0, remain);
    if (accepted.some((file) => !isImageFileName(file.name))) {
      setError("请选择图片文件");
      return;
    }
    if (accepted.some((file) => file.size > MAX_IMAGE_BYTES)) {
      setError("单张图片不能超过 10MB");
      return;
    }

    try {
      const read = await Promise.all(
        accepted.map(async (file) => ({
          id: crypto.randomUUID(),
          fileName: file.name,
          fileSize: file.size,
          content: await readAsBase64(file),
        })),
      );
      setImages((prev) => [...prev, ...read].slice(0, MAX_IMAGE_COUNT));
    } catch {
      setError("图片读取失败，请重试");
    }
  };

  const removeImage = (id: string) => {
    setImages((prev) => prev.filter((img) => img.id !== id));
  };

  // 选择文件：校验大小后立即发送（不限制类型，与图片的暂存流程不同）
  const handleFileChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    e.target.value = "";
    if (!file) return;

    setError("");
    setEmojiOpen(false);
    if (file.size > MAX_FILE_BYTES) {
      setError("文件不能超过 15MB");
      return;
    }

    setUploading(true);
    try {
      const content = await readAsBase64(file);
      const ok = await sendFile(file.name, file.size, content);
      if (!ok) setError("文件发送失败，请重试");
    } catch {
      setError("文件读取失败，请重试");
    } finally {
      setUploading(false);
    }
  };

  return (
    <div className="shrink-0 border-t border-line-light bg-input">
      {error && (
        <div role="alert" className="px-5 pt-2 text-xs text-danger">
          {error}
        </div>
      )}
      {images.length > 0 && (
        <ul aria-label="待发送图片" className="flex flex-wrap gap-2 px-5 pt-3">
          {images.map((img) => (
            <li key={img.id} className="relative">
              <img
                src={`data:${imageMimeType(img.fileName)};base64,${img.content}`}
                alt={img.fileName}
                className="h-16 w-16 rounded-base border border-line object-cover"
              />
              <button
                type="button"
                aria-label={`移除图片 ${img.fileName}`}
                onClick={() => removeImage(img.id)}
                className="absolute -right-1.5 -top-1.5 flex h-4 w-4 items-center justify-center rounded-full border border-line bg-panel text-secondary hover:text-danger"
              >
                <X size={10} strokeWidth={2} />
              </button>
            </li>
          ))}
        </ul>
      )}
      <div className="flex min-h-input items-end px-5 pt-3">
        <textarea
          ref={textareaRef}
          value={text}
          onChange={(e) => setText(e.target.value)}
          onKeyDown={handleKeyDown}
          onCompositionStart={() => {
            composingRef.current = true;
          }}
          onCompositionEnd={() => {
            composingRef.current = false;
          }}
          placeholder="输入消息，Enter 发送，Shift+Enter 换行"
          className="min-h-input flex-1 resize-none bg-transparent text-base text-primary placeholder:text-tertiary focus:outline-none"
        />
      </div>

      <div className="relative flex h-toolbar items-center gap-4 px-5">
        <button
          ref={emojiBtnRef}
          type="button"
          aria-label="表情"
          aria-haspopup="dialog"
          aria-expanded={emojiOpen}
          onClick={() => setEmojiOpen((open) => !open)}
          className={cn("hv-tool", emojiOpen ? "text-brand" : "text-secondary")}
        >
          <Smile size={20} strokeWidth={1.5} />
        </button>
        <button
          type="button"
          aria-label="图片"
          onClick={() => fileInputRef.current?.click()}
          disabled={uploading}
          className={cn(
            "hv-tool text-secondary",
            uploading && "cursor-not-allowed opacity-60",
          )}
        >
          {uploading ? (
            <Loader2 size={20} strokeWidth={1.5} className="animate-spin" />
          ) : (
            <Image size={20} strokeWidth={1.5} />
          )}
        </button>
        <input
          ref={fileInputRef}
          type="file"
          accept="image/png,image/jpeg,image/gif,image/webp,image/bmp"
          multiple
          className="hidden"
          onChange={(e) => void handleImageChange(e)}
        />
        <button
          type="button"
          aria-label="文件"
          onClick={() => fileDocInputRef.current?.click()}
          disabled={uploading}
          className={cn(
            "hv-tool text-secondary",
            uploading && "cursor-not-allowed opacity-60",
          )}
        >
          <Paperclip size={20} strokeWidth={1.5} />
        </button>
        <input
          ref={fileDocInputRef}
          type="file"
          className="hidden"
          onChange={(e) => void handleFileChange(e)}
        />

        <button
          type="button"
          onClick={() => void submit()}
          disabled={!canSend || uploading}
          className="ml-auto h-7 w-[88px] shrink-0 rounded-base text-sm"
          style={{
            background: canSend
              ? "var(--color-brand)"
              : "var(--input-disabled-bg)",
            color: canSend ? "var(--text-on-brand)" : "var(--text-tertiary)",
            cursor: canSend ? "pointer" : "not-allowed",
          }}
        >
          {uploading ? "发送中" : "发送"}
        </button>

        {emojiOpen && (
          <EmojiPicker
            onPick={pickEmoji}
            onClose={() => setEmojiOpen(false)}
            returnFocusRef={emojiBtnRef}
          />
        )}
      </div>
    </div>
  );
}
