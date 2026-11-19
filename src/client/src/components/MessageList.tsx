import { useEffect, useRef } from "react";
import dayjs from "dayjs";
import { useMessageStore } from "@/stores/message";
import { useSendMessage } from "@/hooks/useSendMessage";
import { MessageBubble } from "@/components/MessageBubble";
import { EmptyState } from "@/components/EmptyState";
import type { ChatSessionInfo } from "@/types";

interface MessageListProps {
  session: ChatSessionInfo;
}

const FIVE_MIN = 5 * 60; // 时间分割线阈值：距上一条 >5 分钟插入

function formatDivider(ts: string): string {
  const d = dayjs.unix(Number(ts));
  const today = dayjs().startOf("day");
  if (d.isSame(today, "day")) return d.format("HH:mm");
  if (d.isSame(today.subtract(1, "day"), "day"))
    return `昨天 ${d.format("HH:mm")}`;
  return d.format("MM-DD HH:mm");
}

// 消息区（style.md §2.4）：可滚动，role=log，时间分割线
export function MessageList({ session }: MessageListProps) {
  // selector 必须返回稳定引用（undefined 或 store 内数组），不能每次新建 []，
  // 否则 Zustand v5 getSnapshot 不稳定会触发无限重渲染
  const stored = useMessageStore((s) => s.messages[session.chatSessionId]);
  const messages = stored ?? [];
  const { retry } = useSendMessage(session.chatSessionId);
  const scrollRef = useRef<HTMLDivElement>(null);
  const isGroup = !session.singleChatFriendId;

  // 切换会话 / 新消息后滚动到底部（切换会话后条数可能不变，需带上会话 id）
  useEffect(() => {
    const el = scrollRef.current;
    if (el) el.scrollTop = el.scrollHeight;
  }, [session.chatSessionId, messages.length]);

  if (messages.length === 0) {
    return <EmptyState title="暂无消息" description="发送第一条消息开始聊天" />;
  }

  // 构造带时间分割线的渲染序列
  const nodes: React.ReactNode[] = [];
  let lastTime: number | null = null;
  let lastSender: string | null = null;

  messages.forEach((msg, idx) => {
    const t = Number(msg.createTime);
    const showDivider = lastTime === null || t - lastTime > FIVE_MIN;
    const compact = !showDivider && lastSender === msg.sender.userId && idx > 0;

    if (showDivider) {
      nodes.push(
        <div key={`d-${msg.messageId}`} className="my-3 flex justify-center">
          <span className="text-xs text-tertiary tabular-nums">
            {formatDivider(msg.createTime)}
          </span>
        </div>,
      );
    }
    nodes.push(
      <MessageBubble
        key={msg.messageId}
        msg={msg}
        isGroup={isGroup}
        compact={compact}
        onRetry={retry}
      />,
    );
    lastTime = t;
    lastSender = msg.sender.userId;
  });

  return (
    <div
      ref={scrollRef}
      role="log"
      aria-live="polite"
      className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto bg-app px-5 py-4"
    >
      {nodes}
    </div>
  );
}
