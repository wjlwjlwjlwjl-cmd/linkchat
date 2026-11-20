import { MoreHorizontal } from "lucide-react";
import { useUiStore } from "@/stores/ui";
import { cn } from "@/components/ui/cn";
import type { ChatSessionInfo } from "@/types";

interface MessageHeaderProps {
  session: ChatSessionInfo;
}

// 会话标题栏（style.md §2.3）：高 60px，底色 --bg-header，底部 1px --line-light
export function MessageHeader({ session }: MessageHeaderProps) {
  const infoOpen = useUiStore((s) => s.infoOpen);
  const setInfoOpen = useUiStore((s) => s.setInfoOpen);

  return (
    <header className="flex h-header shrink-0 items-center justify-between border-b border-line-light bg-header px-5">
      <h1 className="text-lg font-medium text-primary">
        {session.chatSessionName}
      </h1>
      <button
        type="button"
        aria-label="聊天信息"
        aria-pressed={infoOpen}
        onClick={() => setInfoOpen(!infoOpen)}
        className={cn(
          "hv-icon rounded-base p-1.5",
          infoOpen ? "text-brand" : "text-secondary",
        )}
      >
        <MoreHorizontal size={20} strokeWidth={1.5} />
      </button>
    </header>
  );
}
