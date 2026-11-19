import { MessageSquareMore } from "lucide-react";

interface EmptyStateProps {
  title?: string;
  description?: string;
}

// 空状态（style.md §2.6）：64px 图标 + 20px 标题 + 13px 说明
export function EmptyState({
  title = "选择一个会话开始聊天",
  description,
}: EmptyStateProps) {
  return (
    <div className="flex h-full w-full flex-col items-center justify-center gap-3 bg-app">
      <MessageSquareMore
        size={64}
        strokeWidth={1.5}
        style={{ color: "var(--empty-icon)" }}
      />
      <h2 className="text-xl text-secondary">{title}</h2>
      {description && <p className="text-sm text-tertiary">{description}</p>}
    </div>
  );
}
