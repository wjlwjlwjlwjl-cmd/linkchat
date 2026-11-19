import { useRef, useState } from "react";
import { Smile, Image, Paperclip } from "lucide-react";
import { useSendMessage } from "@/hooks/useSendMessage";

interface InputAreaProps {
  sessionId: string;
}

// 输入区（style.md §2.5 / §4.8）：Enter 发送，Shift+Enter 换行，
// IME 组合期间 Enter 不发送
export function InputArea({ sessionId }: InputAreaProps) {
  const { send } = useSendMessage(sessionId);
  const [text, setText] = useState("");
  const composingRef = useRef(false);
  const canSend = text.trim().length > 0;

  const submit = async () => {
    if (!canSend) return;
    const ok = await send(text);
    if (ok) setText("");
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key !== "Enter" || e.shiftKey) return;
    // 中文输入法组合期间不发送
    if (composingRef.current || e.nativeEvent.isComposing) return;
    e.preventDefault();
    void submit();
  };

  return (
    <div className="shrink-0 border-t border-line-light bg-input">
      <div className="flex h-toolbar items-center gap-4 px-5">
        <button
          type="button"
          aria-label="表情"
          className="hv-tool text-secondary"
          disabled
        >
          <Smile size={20} strokeWidth={1.5} />
        </button>
        <button
          type="button"
          aria-label="图片"
          className="hv-tool text-secondary"
          disabled
        >
          <Image size={20} strokeWidth={1.5} />
        </button>
        <button
          type="button"
          aria-label="文件"
          className="hv-tool text-secondary"
          disabled
        >
          <Paperclip size={20} strokeWidth={1.5} />
        </button>
      </div>

      <div className="flex min-h-input items-end px-5 pb-3">
        <textarea
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
        <button
          type="button"
          onClick={() => void submit()}
          disabled={!canSend}
          className="mb-1 ml-3 h-7 w-[88px] shrink-0 rounded-base text-sm"
          style={{
            background: canSend
              ? "var(--color-brand)"
              : "var(--input-disabled-bg)",
            color: canSend ? "var(--text-on-brand)" : "var(--text-tertiary)",
            cursor: canSend ? "pointer" : "not-allowed",
          }}
        >
          发送
        </button>
      </div>
    </div>
  );
}
