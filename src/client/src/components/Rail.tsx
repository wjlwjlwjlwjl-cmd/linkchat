import { MessageSquare, Users, Settings } from "lucide-react";
import { useAuthStore } from "@/stores/auth";
import { Avatar } from "@/components/Avatar";

// 左侧功能栏（style.md §2.1）：深色 --bg-rail，宽 60px，图标 24×24
export function Rail() {
  const profile = useAuthStore((s) => s.profile);

  return (
    <nav className="flex h-full w-rail shrink-0 flex-col items-center bg-rail py-3">
      <button type="button" aria-label="我的" className="mb-6">
        <Avatar
          fileId={profile?.avatar}
          name={profile?.nickname ?? "我"}
          size={32}
        />
      </button>

      <div className="flex flex-1 flex-col items-center gap-6">
        <button
          type="button"
          aria-label="消息"
          aria-current="page"
          className="relative rounded-base bg-rail-active p-2 text-inverse"
        >
          <MessageSquare size={24} strokeWidth={1.5} />
          <span
            className="absolute right-1 top-1 h-2 w-2 rounded-full bg-danger"
            style={{ boxShadow: "0 0 0 1.5px var(--bg-rail)" }}
            aria-label="3 条未读"
          />
        </button>
        <button
          type="button"
          aria-label="通讯录"
          className="hv-rail rounded-base p-2"
          style={{ color: "var(--rail-icon)" }}
        >
          <Users size={24} strokeWidth={1.5} />
        </button>
      </div>

      <button
        type="button"
        aria-label="设置"
        className="hv-rail rounded-base p-2"
        style={{ color: "var(--rail-icon)" }}
      >
        <Settings size={24} strokeWidth={1.5} />
      </button>
    </nav>
  );
}
