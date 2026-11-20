import { useEffect, useMemo, useRef, useState } from "react";
import { createPortal } from "react-dom";
import { Loader2, Search, X } from "lucide-react";
import { getChatSessionMembers, searchFriends } from "@/api/friend";
import { useAuthStore } from "@/stores/auth";
import { useSessionStore } from "@/stores/session";
import { Avatar } from "@/components/Avatar";
import { ContactCardPopup } from "@/components/ContactCardPopup";
import { cn } from "@/components/ui/cn";
import type { ChatSessionInfo, UserInfo } from "@/types";

const PANEL_WIDTH = 280;
const SEARCH_DEBOUNCE_MS = 250;

// 把命中的关键词染成品牌色
function Highlight({ text, keyword }: { text: string; keyword: string }) {
  if (!keyword) return <>{text}</>;
  const lower = text.toLowerCase();
  const key = keyword.toLowerCase();
  const parts: { text: string; hit: boolean }[] = [];
  let from = 0;
  let index = lower.indexOf(key, from);
  while (index !== -1) {
    if (index > from) {
      parts.push({ text: text.slice(from, index), hit: false });
    }
    parts.push({ text: text.slice(index, index + key.length), hit: true });
    from = index + key.length;
    index = lower.indexOf(key, from);
  }
  if (from < text.length) parts.push({ text: text.slice(from), hit: false });
  return (
    <>
      {parts.map((part, i) =>
        part.hit ? (
          <span key={i} className="text-brand">
            {part.text}
          </span>
        ) : (
          <span key={i}>{part.text}</span>
        ),
      )}
    </>
  );
}

function SectionTitle({ children }: { children: string }) {
  return <div className="px-3 pb-1 pt-3 text-xs text-tertiary">{children}</div>;
}

interface GroupHit {
  session: ChatSessionInfo;
  memberNames: string;
}

// 侧边栏顶部搜索：输入昵称后浮层展示匹配联系人与包含他们的群聊
export function FriendSearch() {
  const [open, setOpen] = useState(false);
  const [keyword, setKeyword] = useState("");
  const [anchor, setAnchor] = useState<{ left: number; top: number } | null>(
    null,
  );
  const [results, setResults] = useState<UserInfo[]>([]);
  const [loading, setLoading] = useState(false);
  const [membersBySession, setMembersBySession] = useState<
    Record<string, UserInfo[]>
  >({});
  // 点击搜索结果联系人后在行旁弹出联系人卡片
  const [card, setCard] = useState<{
    user: UserInfo;
    rect: DOMRect;
  } | null>(null);

  const wrapRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const panelRef = useRef<HTMLDivElement>(null);
  const searchSeq = useRef(0);

  const sessions = useSessionStore((s) => s.sessions);
  const setActive = useSessionStore((s) => s.setActive);
  const myId = useAuthStore((s) => s.profile?.userId);

  const groups = useMemo(
    () => sessions.filter((s) => !s.singleChatFriendId),
    [sessions],
  );

  const updateAnchor = () => {
    const rect = wrapRef.current?.getBoundingClientRect();
    if (!rect) return;
    setAnchor({ left: rect.left, top: rect.bottom + 8 });
  };

  const close = () => {
    setOpen(false);
    setKeyword("");
    setResults([]);
    setCard(null);
  };

  // 打开浮层并按需加载群成员（用于判断「包含搜索到的联系人」）
  useEffect(() => {
    if (!open) return;
    const missing = groups.filter((g) => !membersBySession[g.chatSessionId]);
    if (missing.length === 0) return;
    let cancelled = false;
    void Promise.all(
      missing.map(async (g) => {
        try {
          const rsp = await getChatSessionMembers(g.chatSessionId);
          return [g.chatSessionId, rsp.memberInfoList ?? []] as const;
        } catch {
          return [g.chatSessionId, [] as UserInfo[]] as const;
        }
      }),
    ).then((entries) => {
      if (cancelled) return;
      setMembersBySession((prev) => {
        const next = { ...prev };
        entries.forEach(([id, list]) => {
          next[id] = list;
        });
        return next;
      });
    });
    return () => {
      cancelled = true;
    };
  }, [open, groups, membersBySession]);

  // 关键词变化后防抖搜索
  useEffect(() => {
    if (!open) return;
    const key = keyword.trim();
    if (!key) {
      searchSeq.current += 1;
      return;
    }
    const seq = ++searchSeq.current;
    const timer = window.setTimeout(() => {
      setLoading(true);
      searchFriends(key)
        .then((rsp) => {
          if (seq !== searchSeq.current) return;
          setResults(rsp.success === true ? (rsp.userInfo ?? []) : []);
          setLoading(false);
        })
        .catch(() => {
          if (seq !== searchSeq.current) return;
          setResults([]);
          setLoading(false);
        });
    }, SEARCH_DEBOUNCE_MS);
    return () => window.clearTimeout(timer);
  }, [keyword, open]);

  // 点击浮层与输入框之外的区域关闭；Esc 关闭
  useEffect(() => {
    if (!open) return;
    const onPointerDown = (e: MouseEvent) => {
      const target = e.target as Node;
      if (panelRef.current?.contains(target)) return;
      if (wrapRef.current?.contains(target)) return;
      // 联系人卡片是独立 portal，交互时不能关掉搜索浮层
      if ((e.target as HTMLElement).closest("[data-contact-card]")) return;
      close();
    };
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape") close();
    };
    const onResize = () => updateAnchor();
    document.addEventListener("mousedown", onPointerDown);
    document.addEventListener("keydown", onKeyDown);
    window.addEventListener("resize", onResize);
    return () => {
      document.removeEventListener("mousedown", onPointerDown);
      document.removeEventListener("keydown", onKeyDown);
      window.removeEventListener("resize", onResize);
    };
  }, [open]);

  const trimmed = keyword.trim();
  const contacts = results.filter((u) => u.userId !== myId);
  const contactIds = new Set(contacts.map((u) => u.userId));

  const groupHits: GroupHit[] = [];
  if (trimmed) {
    groups.forEach((session) => {
      const members = membersBySession[session.chatSessionId];
      if (!members) return;
      if (!members.some((m) => contactIds.has(m.userId))) return;
      const memberNames = members
        .filter((m) => m.userId !== myId)
        .map((m) => m.nickname || "未命名")
        .join("、");
      groupHits.push({ session, memberNames });
    });
  }

  const empty =
    trimmed !== "" &&
    !loading &&
    contacts.length === 0 &&
    groupHits.length === 0;

  const openPanel = () => {
    updateAnchor();
    setOpen(true);
  };

  const panel =
    open && anchor
      ? createPortal(
          <div
            ref={panelRef}
            role="dialog"
            aria-label="搜索联系人与群聊"
            className="fixed z-50 flex flex-col overflow-hidden rounded-lg border border-line bg-panel shadow-popup"
            style={{
              left: anchor.left,
              top: anchor.top,
              width: PANEL_WIDTH,
              maxHeight: `calc(100vh - ${anchor.top + 16}px)`,
            }}
          >
            <div className="scroll-area min-h-0 flex-1 overflow-y-auto pb-2">
              {trimmed === "" && (
                <div className="px-3 py-6 text-center text-xs text-tertiary">
                  输入昵称搜索联系人
                </div>
              )}
              {trimmed !== "" && loading && contacts.length === 0 && (
                <div className="flex items-center justify-center gap-2 px-3 py-6 text-xs text-tertiary">
                  <Loader2
                    size={14}
                    className="animate-spin"
                    aria-hidden="true"
                  />
                  搜索中…
                </div>
              )}
              {empty && (
                <div className="px-3 py-6 text-center text-xs text-tertiary">
                  未找到相关联系人
                </div>
              )}

              {contacts.length > 0 && (
                <>
                  <SectionTitle>联系人</SectionTitle>
                  {contacts.map((user) => {
                    const name = user.nickname || "未命名";
                    return (
                      <button
                        key={user.userId}
                        type="button"
                        aria-haspopup="dialog"
                        onClick={(e) => {
                          const rect = e.currentTarget.getBoundingClientRect();
                          setCard((prev) =>
                            prev?.user.userId === user.userId
                              ? null
                              : { user, rect },
                          );
                        }}
                        className="hv-list flex w-full items-center gap-3 px-3 py-2 text-left"
                      >
                        <Avatar fileId={user.avatar} name={name} />
                        <span className="min-w-0 flex-1 truncate text-sm text-primary">
                          <Highlight text={name} keyword={trimmed} />
                        </span>
                      </button>
                    );
                  })}
                </>
              )}

              {groupHits.length > 0 && (
                <>
                  <SectionTitle>群聊</SectionTitle>
                  {groupHits.map(({ session, memberNames }) => {
                    const name = session.chatSessionName || "未命名会话";
                    return (
                      <button
                        key={session.chatSessionId}
                        type="button"
                        onClick={() => {
                          setActive(session.chatSessionId);
                          close();
                        }}
                        className="hv-list flex w-full items-center gap-3 px-3 py-2 text-left"
                      >
                        <Avatar fileId={session.avatar} name={name} />
                        <span className="min-w-0 flex-1">
                          <span className="block truncate text-sm text-primary">
                            <Highlight text={name} keyword={trimmed} />
                          </span>
                          <span className="block truncate text-xs text-tertiary">
                            包含：{memberNames}
                          </span>
                        </span>
                      </button>
                    );
                  })}
                </>
              )}
            </div>
          </div>,
          document.body,
        )
      : null;

  return (
    <div ref={wrapRef} className="shrink-0 px-3 py-2">
      <div
        className={cn(
          "flex h-8 items-center rounded-base px-2",
          open && "ring-1 ring-brand",
        )}
        style={{ background: "var(--search-bg)" }}
      >
        <Search size={16} strokeWidth={1.5} className="text-secondary" />
        <input
          ref={inputRef}
          type="text"
          value={keyword}
          onChange={(e) => {
            const value = e.target.value;
            setKeyword(value);
            // 清空时直接复位，避免沿用上一次结果
            if (value.trim() === "") {
              searchSeq.current += 1;
              setResults([]);
              setLoading(false);
            }
          }}
          onFocus={openPanel}
          onClick={() => {
            if (!open) openPanel();
          }}
          placeholder="搜索"
          className="ml-2 w-full bg-transparent text-sm text-primary placeholder:text-tertiary focus:outline-none"
        />
        {keyword !== "" && (
          <button
            type="button"
            aria-label="清空搜索"
            onClick={() => {
              setKeyword("");
              inputRef.current?.focus();
            }}
            className="text-tertiary hover:text-secondary"
          >
            <X size={14} />
          </button>
        )}
      </div>
      {panel}
      {open && card && (
        <ContactCardPopup
          user={card.user}
          anchor={card.rect}
          onClose={() => setCard(null)}
        />
      )}
    </div>
  );
}
