import { useEffect, useRef, useState } from "react";
import {
  EMOJIS,
  EMOJI_RECENT_MAX,
  loadRecentEmojis,
  saveRecentEmojis,
  type EmojiItem,
} from "@/components/emojiData";

interface EmojiPickerProps {
  onPick: (emoji: EmojiItem) => void;
  onClose: () => void;
  returnFocusRef: React.RefObject<HTMLButtonElement>;
}

const FOCUSABLE =
  'button:not([disabled]), input:not([disabled]), textarea:not([disabled]), [tabindex]:not([tabindex="-1"])';

// 表情面板：最近使用 + 所有表情（Fluent Emoji 3D，资源在 public/emoji）
export function EmojiPicker({
  onPick,
  onClose,
  returnFocusRef,
}: EmojiPickerProps) {
  const [recent, setRecent] = useState<string[]>(loadRecentEmojis);
  const rootRef = useRef<HTMLDivElement>(null);

  // 打开时焦点进入面板
  useEffect(() => {
    rootRef.current?.focus();
  }, []);

  // Esc 关闭并把焦点还给表情按钮，点击外部关闭；Tab 在面板内循环
  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
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
      if (e.key === "Escape") {
        onClose();
        returnFocusRef.current?.focus();
      }
    };
    const onPointerDown = (e: MouseEvent) => {
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
  }, [onClose, returnFocusRef]);

  const handlePick = (emoji: EmojiItem) => {
    onPick(emoji);
    setRecent((prev) => {
      const next = [
        emoji.slug,
        ...prev.filter((slug) => slug !== emoji.slug),
      ].slice(0, EMOJI_RECENT_MAX);
      saveRecentEmojis(next);
      return next;
    });
  };

  const recentItems = recent
    .map((slug) => EMOJIS.find((item) => item.slug === slug))
    .filter((item): item is EmojiItem => item !== undefined);

  const renderGrid = (items: EmojiItem[]) => (
    <div className="grid grid-cols-8 gap-1">
      {items.map((emoji) => (
        <button
          key={emoji.slug}
          type="button"
          title={emoji.label}
          aria-label={emoji.label}
          onClick={() => handlePick(emoji)}
          className="hv-icon flex h-9 w-9 items-center justify-center rounded-base"
        >
          <img
            src={`${import.meta.env.BASE_URL}emoji/${emoji.slug}.png`}
            alt=""
            width={28}
            height={28}
            loading="lazy"
            draggable={false}
            className="h-7 w-7"
          />
        </button>
      ))}
    </div>
  );

  return (
    <div
      ref={rootRef}
      role="dialog"
      aria-label="表情"
      tabIndex={-1}
      className="scroll-area absolute bottom-full left-5 z-20 mb-2 max-h-[360px] w-[340px] overflow-y-auto rounded-lg border border-line bg-panel p-3 shadow-popup focus:outline-none"
    >
      {recentItems.length > 0 && (
        <section className="mb-3">
          <h3 className="mb-1 px-1 text-xs text-tertiary">最近使用</h3>
          {renderGrid(recentItems)}
        </section>
      )}
      <section>
        <h3 className="mb-1 px-1 text-xs text-tertiary">所有表情</h3>
        {renderGrid(EMOJIS)}
      </section>
    </div>
  );
}
