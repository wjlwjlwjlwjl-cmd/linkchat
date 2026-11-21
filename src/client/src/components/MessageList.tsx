import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import dayjs from "dayjs";
import { getHistory } from "@/api/message";
import { useMessageStore } from "@/stores/message";
import { requestFileUrl } from "@/lib/fileUrlBatch";
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
  const messages = useMemo(() => stored ?? [], [stored]);
  // 历史消息中的文件 ID 序列（字符串，作为批量预签名 effect 的依赖）
  const fileIdsKey = messages
    .map((m) => {
      const file = m.message.fileMessage;
      return file?.fileId && !file.fileContent ? file.fileId : "";
    })
    .filter(Boolean)
    .join(",");
  // 最后一条消息发送失败时，在消息区末尾居中展示原因
  const lastMessage = messages[messages.length - 1];
  const lastFailReason =
    lastMessage?.status === "failed" ? lastMessage.failReason : undefined;
  const { retry } = useSendMessage(session.chatSessionId);
  const setMessages = useMessageStore((s) => s.setMessages);
  const scrollRef = useRef<HTMLDivElement>(null);
  const isGroup = !session.singleChatFriendId;

  // 分页状态：会话切换时由父组件通过 key 重建组件
  const [loadingOlder, setLoadingOlder] = useState(false);
  const [hasMore, setHasMore] = useState(true);
  const pendingPrependRef = useRef(false); // 本次消息增长来自上滑加载，需要保持滚动位置
  const prevScrollRef = useRef({ height: 0, top: 0 });

  // 切换会话 / 新消息 / 出现失败提示后滚动到底部；上滑加载时改为保持原位置
  useEffect(() => {
    const el = scrollRef.current;
    if (!el) return;
    if (pendingPrependRef.current) {
      pendingPrependRef.current = false;
      const prev = prevScrollRef.current;
      el.scrollTop = el.scrollHeight - prev.height + prev.top;
      return;
    }
    el.scrollTop = el.scrollHeight;
  }, [session.chatSessionId, messages.length, lastFailReason]);

  const loadOlder = useCallback(async () => {
    const anchor = messages[0];
    if (loadingOlder || !hasMore || !anchor) return;
    setLoadingOlder(true);
    // 以最早一条消息所在分钟的前一分钟为区间右界，逐步放大时间窗口直到取到消息
    const overMinute = Math.floor(Number(anchor.createTime) / 60) - 1;
    let windowMinutes = 7 * 24 * 60;
    try {
      for (let i = 0; i < 3; i += 1) {
        const rsp = await getHistory({
          chatSessionId: session.chatSessionId,
          startTime: (overMinute - windowMinutes) * 60,
          overTime: overMinute * 60,
        });
        const list = rsp.success === true ? (rsp.msgList ?? []) : [];
        if (list.length > 0) {
          const el = scrollRef.current;
          if (el) {
            prevScrollRef.current = {
              height: el.scrollHeight,
              top: el.scrollTop,
            };
          }
          pendingPrependRef.current = true;
          setMessages(session.chatSessionId, list);
          return;
        }
        windowMinutes *= 30;
      }
      setHasMore(false);
    } catch {
      // 网络失败保留 hasMore，下次滑到顶部可重试
    } finally {
      setLoadingOlder(false);
    }
  }, [hasMore, loadingOlder, messages, session.chatSessionId, setMessages]);

  // 历史消息里的文件与页面上的头像统一走批量预签名；手动下载仍走单个接口
  useEffect(() => {
    if (!fileIdsKey) return;
    new Set(fileIdsKey.split(",")).forEach(requestFileUrl);
  }, [fileIdsKey]);

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
      onScroll={(e) => {
        if (e.currentTarget.scrollTop <= 8) void loadOlder();
      }}
      className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto bg-app px-5 py-4"
    >
      {loadingOlder && (
        <div className="mb-2 text-center text-xs text-secondary">加载中…</div>
      )}
      {nodes}
      {lastFailReason && (
        <div
          role="alert"
          className="mt-4 px-6 text-center text-xs leading-5 text-danger"
        >
          {lastFailReason}
        </div>
      )}
    </div>
  );
}
