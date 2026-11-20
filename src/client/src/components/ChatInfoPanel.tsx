import { useEffect, useState } from "react";
import { ChevronRight, Loader2, X } from "lucide-react";
import { getChatSessionMembers } from "@/api/friend";
import { useSessionStore } from "@/stores/session";
import { Avatar } from "@/components/Avatar";
import { MessageSearchDialog } from "@/components/MessageSearchDialog";
import { UserInfoPopup } from "@/components/UserInfoPopup";
import { cn } from "@/components/ui/cn";
import type { ChatSessionInfo, UserInfo } from "@/types";

interface ChatInfoPanelProps {
  session: ChatSessionInfo;
  onClose: () => void;
}

// 置顶聊天开关（无后端字段，按账号存本地）
function PinSwitch({ sessionId, label }: { sessionId: string; label: string }) {
  const pinned = useSessionStore((s) => s.pinnedIds.includes(sessionId));
  const togglePin = useSessionStore((s) => s.togglePin);

  return (
    <button
      type="button"
      role="switch"
      aria-checked={pinned}
      aria-label={label}
      onClick={() => togglePin(sessionId)}
      className={cn(
        "relative h-5 w-9 shrink-0 rounded-full transition-colors",
        pinned ? "bg-brand" : "bg-line-light",
      )}
    >
      <span
        className={cn(
          "absolute left-0.5 top-0.5 h-4 w-4 rounded-full bg-[var(--text-inverse)] transition-transform",
          pinned ? "translate-x-4" : "translate-x-0",
        )}
      />
    </button>
  );
}

// 聊天信息：单聊展示对方头像，群聊展示成员网格，下方为查找记录 / 置顶聊天
export function ChatInfoPanel({ session, onClose }: ChatInfoPanelProps) {
  const isGroup = !session.singleChatFriendId;
  const [members, setMembers] = useState<UserInfo[]>([]);
  const [loading, setLoading] = useState(isGroup);
  const [searchOpen, setSearchOpen] = useState(false);
  const [infoAnchor, setInfoAnchor] = useState<{
    user: UserInfo;
    rect: DOMRect;
  } | null>(null);

  useEffect(() => {
    if (!isGroup) return;
    let cancelled = false;
    getChatSessionMembers(session.chatSessionId)
      .then((rsp) => {
        if (cancelled) return;
        setMembers(rsp.success === true ? (rsp.memberInfoList ?? []) : []);
        setLoading(false);
      })
      .catch(() => {
        if (cancelled) return;
        setMembers([]);
        setLoading(false);
      });
    return () => {
      cancelled = true;
    };
  }, [isGroup, session.chatSessionId]);

  return (
    <div className="flex h-full flex-col bg-list">
      <header className="flex h-header shrink-0 items-center justify-between border-b border-line-light px-4">
        <span className="text-sm font-medium text-primary">聊天信息</span>
        <button
          type="button"
          aria-label="关闭聊天信息"
          onClick={onClose}
          className="hv-icon rounded-base p-1.5 text-secondary"
        >
          <X size={18} strokeWidth={1.5} />
        </button>
      </header>

      <div className="scroll-area min-h-0 flex-1 overflow-y-auto overscroll-contain">
        <section className="border-b border-line-light px-4 py-6">
          {isGroup ? (
            loading ? (
              <div className="flex items-center justify-center gap-2 py-4 text-xs text-tertiary">
                <Loader2
                  size={14}
                  className="animate-spin"
                  aria-hidden="true"
                />
                加载成员…
              </div>
            ) : members.length === 0 ? (
              <div className="py-4 text-center text-xs text-tertiary">
                暂无成员信息
              </div>
            ) : (
              <div className="grid grid-cols-4 gap-x-2 gap-y-4">
                {members.map((member) => {
                  const name = member.nickname || "未命名";
                  return (
                    <button
                      key={member.userId}
                      type="button"
                      data-user-avatar
                      aria-label={`查看 ${name} 的资料`}
                      onClick={(e) =>
                        setInfoAnchor({
                          user: member,
                          rect: e.currentTarget.getBoundingClientRect(),
                        })
                      }
                      className="flex min-w-0 cursor-pointer flex-col items-center gap-1.5"
                    >
                      <Avatar fileId={member.avatar} name={name} size={44} />
                      <span className="w-full truncate text-center text-xs text-secondary">
                        {name}
                      </span>
                    </button>
                  );
                })}
              </div>
            )
          ) : (
            <button
              type="button"
              data-user-avatar
              aria-label={`查看 ${session.chatSessionName} 的资料`}
              onClick={(e) => {
                if (!session.singleChatFriendId) return;
                setInfoAnchor({
                  user: {
                    userId: session.singleChatFriendId,
                    nickname: session.chatSessionName,
                    avatar: session.avatar,
                  },
                  rect: e.currentTarget.getBoundingClientRect(),
                });
              }}
              className="flex cursor-pointer flex-col items-start gap-1.5"
            >
              <Avatar
                fileId={session.avatar}
                name={session.chatSessionName}
                size={44}
              />
              <span className="max-w-full truncate text-xs text-secondary">
                {session.chatSessionName}
              </span>
            </button>
          )}
        </section>

        <div className="border-b border-line-light">
          <button
            type="button"
            onClick={() => setSearchOpen(true)}
            className="hv-list flex h-12 w-full items-center justify-between px-4 text-sm text-primary"
          >
            <span>查找聊天记录</span>
            <ChevronRight
              size={18}
              strokeWidth={1.5}
              className="text-tertiary"
            />
          </button>
        </div>

        <div className="flex h-12 items-center justify-between px-4">
          <span className="text-sm text-primary">置顶聊天</span>
          <PinSwitch sessionId={session.chatSessionId} label="置顶聊天" />
        </div>
      </div>

      {infoAnchor && (
        <UserInfoPopup
          userId={infoAnchor.user.userId}
          anchor={infoAnchor.rect}
          initial={infoAnchor.user}
          onClose={() => setInfoAnchor(null)}
        />
      )}
      {searchOpen && (
        <MessageSearchDialog
          session={session}
          onClose={() => setSearchOpen(false)}
        />
      )}
    </div>
  );
}
