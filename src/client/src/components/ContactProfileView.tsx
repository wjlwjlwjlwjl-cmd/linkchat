import { useState } from "react";
import { Loader2, MessageCircle } from "lucide-react";
import { getChatSessionList, getSingleChatSession } from "@/api/friend";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import type { UserInfo } from "@/types";

interface ContactProfileViewProps {
  contact: UserInfo;
}

// 资料行：左侧标签 + 右侧内容
function InfoRow({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex gap-4 text-sm leading-6">
      <span className="w-16 shrink-0 text-secondary">{label}</span>
      <span className="min-w-0 break-all text-primary">{value}</span>
    </div>
  );
}

// 好友资料页：展示在聊天详情区，底部「发消息」进入单聊
export function ContactProfileView({ contact }: ContactProfileViewProps) {
  const setSessions = useSessionStore((s) => s.setSessions);
  const setActive = useSessionStore((s) => s.setActive);
  const setSideView = useUiStore((s) => s.setSideView);
  const setSelectedContact = useUiStore((s) => s.setSelectedContact);

  const [opening, setOpening] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const name = contact.nickname || "未命名";

  // 先确保单聊会话存在，再刷新会话列表并进入聊天
  const handleMessage = async () => {
    if (opening) return;
    setOpening(true);
    setError(null);
    try {
      const rsp = await getSingleChatSession(contact.userId);
      const sessionId = rsp.chatSessionId;
      if (rsp.success !== true || !sessionId) {
        setError(rsp.errmsg || "打开会话失败");
        return;
      }
      // 新建的会话不在本地缓存里，刷新列表后再选中
      const listRsp = await getChatSessionList();
      if (listRsp.success === true) {
        setSessions(listRsp.chatSessionInfoList ?? []);
      }
      setActive(sessionId);
      setSelectedContact(null);
      setSideView("chats");
    } catch (err: unknown) {
      setError(err instanceof Error ? err.message : "打开会话失败");
    } finally {
      setOpening(false);
    }
  };

  return (
    <div className="flex h-full flex-col bg-app">
      <div className="scroll-area scroll-contain min-h-0 flex-1 overflow-y-auto">
        <div className="mx-auto w-full max-w-[560px] px-8 pb-10 pt-16">
          <div className="flex items-start gap-5">
            <Avatar fileId={contact.avatar} name={name} size={80} />
            <div className="min-w-0 pt-1">
              <h2 className="truncate text-xl text-primary">{name}</h2>
              <p className="mt-2 truncate text-xs text-tertiary">
                {contact.email || "未填写邮箱"}
              </p>
            </div>
          </div>

          <section className="mt-10">
            <h3 className="text-xs text-tertiary">朋友资料</h3>
            <div className="mt-3 space-y-2">
              <InfoRow label="昵称" value={name} />
              <InfoRow
                label="签名"
                value={contact.description || "这个人很懒，什么都没写"}
              />
            </div>
          </section>

          <div className="my-8 h-px bg-line" />

          <section>
            <h3 className="text-xs text-tertiary">更多信息</h3>
            <div className="mt-3 space-y-2">
              <InfoRow label="邮箱" value={contact.email || "未填写"} />
              <InfoRow label="用户ID" value={contact.userId} />
            </div>
          </section>
        </div>
      </div>

      <div className="shrink-0 px-6 pb-10 pt-4">
        <div className="flex flex-col items-center gap-1.5">
          <button
            type="button"
            onClick={() => void handleMessage()}
            disabled={opening}
            className="flex flex-col items-center gap-1.5 rounded-lg px-4 py-2 text-xs text-brand disabled:opacity-60"
          >
            {opening ? (
              <Loader2 size={26} className="animate-spin" aria-hidden="true" />
            ) : (
              <MessageCircle size={26} strokeWidth={1.5} />
            )}
            <span>{opening ? "正在打开…" : "发消息"}</span>
          </button>
          {error && <p className="text-xs text-danger">{error}</p>}
        </div>
      </div>
    </div>
  );
}
