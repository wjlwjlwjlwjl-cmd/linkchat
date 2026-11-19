import { Search, MoreHorizontal } from "lucide-react";
import type { ChatSessionInfo } from "@/types";

interface MessageHeaderProps {
  session: ChatSessionInfo;
}

// 会话标题栏（style.md §2.3）：高 60px，底色 --bg-header，底部 1px --line-light
export function MessageHeader({ session }: MessageHeaderProps) {
  return (
    <header className="flex h-header shrink-0 items-center justify-between border-b border-line-light bg-header px-5">
      <h1 className="text-lg font-medium text-primary">
        {session.chatSessionName}
      </h1>
      <div className="flex items-center gap-1">
        <button
          type="button"
          aria-label="搜索消息"
          className="hv-icon rounded-base p-1.5 text-secondary"
        >
          <Search size={20} strokeWidth={1.5} />
        </button>
        <button
          type="button"
          aria-label="更多"
          className="hv-icon rounded-base p-1.5 text-secondary"
        >
          <MoreHorizontal size={20} strokeWidth={1.5} />
        </button>
      </div>
    </header>
  );
}
