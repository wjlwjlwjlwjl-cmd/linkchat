import { useEffect, useRef, useState } from "react";
import {
  Check,
  ChevronDown,
  LogOut,
  MessageSquare,
  Monitor,
  Moon,
  Palette,
  Settings,
  Sun,
  Users,
} from "lucide-react";
import { logout } from "@/api/user";
import { useAuthStore } from "@/stores/auth";
import { useMessageStore } from "@/stores/message";
import { hasUnread, useSessionStore } from "@/stores/session";
import { useUiStore, type ThemeMode } from "@/stores/ui";
import { Avatar } from "@/components/Avatar";
import { ProfilePopover } from "@/components/ProfilePopover";
import { cn } from "@/components/ui/cn";

const THEME_OPTIONS: { value: ThemeMode; label: string; icon: typeof Sun }[] = [
  { value: "light", label: "浅色", icon: Sun },
  { value: "dark", label: "深色", icon: Moon },
  { value: "system", label: "跟随系统", icon: Monitor },
];

// 左侧功能栏（style.md §2.1）：深色 --bg-rail，宽 60px，图标 24×24
export function Rail() {
  const profile = useAuthStore((s) => s.profile);
  const themeMode = useUiStore((s) => s.themeMode);
  const setThemeMode = useUiStore((s) => s.setThemeMode);
  const sideView = useUiStore((s) => s.sideView);
  const setSideView = useUiStore((s) => s.setSideView);
  const sessions = useSessionStore((s) => s.sessions);
  const readCursors = useSessionStore((s) => s.readCursors);
  const activeSessionId = useSessionStore((s) => s.activeSessionId);
  // 与会话列表一致：当前打开会话不算未读，其余有未读才显示红点
  const hasAnyUnread = sessions.some(
    (s) =>
      s.chatSessionId !== activeSessionId &&
      hasUnread(s, readCursors[s.chatSessionId]),
  );
  const [menuOpen, setMenuOpen] = useState(false);
  const [themeExpanded, setThemeExpanded] = useState(false);
  const [loggingOut, setLoggingOut] = useState(false);
  const [profileOpen, setProfileOpen] = useState(false);
  const menuRef = useRef<HTMLDivElement>(null);
  const triggerRef = useRef<HTMLButtonElement>(null);
  const profileTriggerRef = useRef<HTMLButtonElement>(null);

  // 弹层：Esc 关闭并回到触发按钮，点击外部关闭（style.md §3.3）
  useEffect(() => {
    if (!menuOpen) return;
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key !== "Escape") return;
      setMenuOpen(false);
      triggerRef.current?.focus();
    };
    const onPointerDown = (e: MouseEvent) => {
      const target = e.target as Node;
      if (
        menuRef.current?.contains(target) ||
        triggerRef.current?.contains(target)
      ) {
        return;
      }
      setMenuOpen(false);
    };
    window.addEventListener("keydown", onKeyDown);
    window.addEventListener("mousedown", onPointerDown);
    return () => {
      window.removeEventListener("keydown", onKeyDown);
      window.removeEventListener("mousedown", onPointerDown);
    };
  }, [menuOpen]);

  const selectTheme = (value: ThemeMode) => {
    setThemeMode(value);
    setMenuOpen(false);
    setThemeExpanded(false);
    triggerRef.current?.focus();
  };

  // 退出登录：先注销服务端登录态，再清空本地会话与消息缓存
  const handleLogout = async () => {
    if (loggingOut) return;
    setLoggingOut(true);
    try {
      await logout();
    } catch {
      // 网络异常也继续本地退出，避免残留登录态
    }
    useAuthStore.getState().clear();
    useSessionStore.setState({
      sessions: [],
      activeSessionId: null,
      sessionsLoaded: false,
      readCursors: {},
    });
    useMessageStore.setState({ messages: {} });
    setMenuOpen(false);
  };

  return (
    <nav className="flex h-full w-rail shrink-0 flex-col items-center bg-rail py-3">
      <div className="relative mb-6">
        {profileOpen && (
          <div className="absolute left-full top-0 z-30 ml-4">
            <ProfilePopover
              onClose={() => setProfileOpen(false)}
              returnFocusRef={profileTriggerRef}
            />
          </div>
        )}
        <button
          ref={profileTriggerRef}
          type="button"
          aria-label="我的"
          aria-haspopup="dialog"
          aria-expanded={profileOpen}
          onClick={() => {
            setProfileOpen((open) => !open);
            setMenuOpen(false);
          }}
        >
          <Avatar
            fileId={profile?.avatar}
            name={profile?.nickname ?? "我"}
            size={32}
          />
        </button>
      </div>

      <div className="flex flex-1 flex-col items-center gap-6">
        <button
          type="button"
          aria-label="消息"
          aria-current={sideView === "chats" ? "page" : undefined}
          onClick={() => setSideView("chats")}
          className={cn(
            "relative rounded-base p-2",
            sideView === "chats" ? "bg-rail-active text-inverse" : "hv-rail",
          )}
          style={
            sideView === "chats" ? undefined : { color: "var(--rail-icon)" }
          }
        >
          <MessageSquare size={24} strokeWidth={1.5} />
          {hasAnyUnread && (
            <span
              className="absolute right-1 top-1 h-2 w-2 rounded-full bg-danger"
              style={{ boxShadow: "0 0 0 1.5px var(--bg-rail)" }}
              aria-label="有未读消息"
            />
          )}
        </button>
        <button
          type="button"
          aria-label="通讯录"
          aria-current={sideView === "contacts" ? "page" : undefined}
          onClick={() => setSideView("contacts")}
          className={cn(
            "rounded-base p-2",
            sideView === "contacts" ? "bg-rail-active text-inverse" : "hv-rail",
          )}
          style={
            sideView === "contacts" ? undefined : { color: "var(--rail-icon)" }
          }
        >
          <Users size={24} strokeWidth={1.5} />
        </button>
      </div>

      <div className="relative">
        {menuOpen && (
          <div
            ref={menuRef}
            role="menu"
            aria-label="设置"
            className="absolute bottom-11 left-0 z-20 w-48 rounded-lg border border-line bg-panel p-1.5 shadow-popup"
          >
            <button
              type="button"
              role="menuitem"
              aria-expanded={themeExpanded}
              onClick={() => setThemeExpanded((expanded) => !expanded)}
              className="hv-icon flex h-9 w-full items-center gap-2.5 rounded-base px-2 text-sm text-primary"
            >
              <Palette size={18} strokeWidth={1.5} />
              <span className="flex-1 text-left">外观</span>
              <ChevronDown
                size={14}
                strokeWidth={1.5}
                className={cn(
                  "transition-transform",
                  themeExpanded && "rotate-180",
                )}
              />
            </button>

            {themeExpanded && (
              <div role="group" aria-label="外观">
                {THEME_OPTIONS.map(({ value, label, icon: Icon }) => (
                  <button
                    key={value}
                    type="button"
                    role="menuitemradio"
                    aria-checked={themeMode === value}
                    onClick={() => selectTheme(value)}
                    className={cn(
                      "flex h-9 w-full items-center gap-2.5 rounded-base pl-6 pr-2 text-sm",
                      themeMode === value
                        ? "text-brand"
                        : "hv-icon text-primary",
                    )}
                  >
                    <Icon size={16} strokeWidth={1.5} />
                    <span className="flex-1 text-left">{label}</span>
                    {themeMode === value && <Check size={14} strokeWidth={2} />}
                  </button>
                ))}
              </div>
            )}

            <div className="my-1 h-px bg-line-light" />

            <button
              type="button"
              role="menuitem"
              onClick={() => void handleLogout()}
              disabled={loggingOut}
              className="flex h-9 w-full items-center gap-2.5 rounded-base px-2 text-sm text-danger hover:bg-danger-weak disabled:opacity-60"
            >
              <LogOut size={18} strokeWidth={1.5} />
              <span>{loggingOut ? "退出中…" : "退出登录"}</span>
            </button>
          </div>
        )}
        <button
          ref={triggerRef}
          type="button"
          aria-label="设置"
          aria-haspopup="menu"
          aria-expanded={menuOpen}
          onClick={() => {
            if (!menuOpen) setThemeExpanded(false);
            setMenuOpen((open) => !open);
            setProfileOpen(false);
          }}
          className="hv-rail rounded-base p-2"
          style={{ color: "var(--rail-icon)" }}
        >
          <Settings size={24} strokeWidth={1.5} />
        </button>
      </div>
    </nav>
  );
}
