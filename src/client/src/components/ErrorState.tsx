import { AlertCircle } from "lucide-react";

interface ErrorStateProps {
  message: string;
  onRetry?: () => void;
}

// 错误态（AGENTS.md §9：文案统一经 mapError / 后端 errmsg）
export function ErrorState({ message, onRetry }: ErrorStateProps) {
  return (
    <div className="flex h-full w-full flex-col items-center justify-center gap-3 bg-app">
      <AlertCircle
        size={64}
        strokeWidth={1.5}
        style={{ color: "var(--empty-icon)" }}
      />
      <p className="text-base text-secondary">{message}</p>
      {onRetry && (
        <button
          type="button"
          onClick={onRetry}
          className="rounded-base border border-line bg-panel px-4 py-1.5 text-sm text-primary"
        >
          重试
        </button>
      )}
    </div>
  );
}
