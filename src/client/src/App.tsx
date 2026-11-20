import { useEffect, useState } from "react";
import { ChatLayout } from "@/features/chat/ChatLayout";
import { LoginPage } from "@/features/auth/LoginPage";
import { MinWidthGuard } from "@/components/MinWidthGuard";
import { useAuthStore } from "@/stores/auth";
import { useSessionStore } from "@/stores/session";
import { useUiStore } from "@/stores/ui";
import { connectWs, disconnectWs } from "@/ws/client";

// 根组件：登录守卫 → 窗口宽度守卫 → 三栏布局
export default function App() {
  const token = useAuthStore((s) => s.token);
  const sessionsLoaded = useSessionStore((s) => s.sessionsLoaded);
  const [tooNarrow, setTooNarrow] = useState(
    typeof window !== "undefined" ? window.innerWidth < 1024 : false,
  );

  useEffect(() => {
    const onResize = () => setTooNarrow(window.innerWidth < 1024);
    window.addEventListener("resize", onResize);
    return () => window.removeEventListener("resize", onResize);
  }, []);

  // 跟随系统：系统深浅色变化时立即切换（style.md §5）
  useEffect(() => {
    const media = window.matchMedia("(prefers-color-scheme: dark)");
    const onChange = () => useUiStore.getState().syncSystemTheme();
    media.addEventListener("change", onChange);
    useUiStore.getState().syncSystemTheme();
    return () => media.removeEventListener("change", onChange);
  }, []);

  // 串行化：先渲染会话列表，再建立 WS；token 清空（退出 / 401）时断开并复位
  useEffect(() => {
    if (!token) {
      disconnectWs();
      useSessionStore.getState().setSessionsLoaded(false);
      useUiStore.getState().setWsError(false);
      return;
    }
    if (sessionsLoaded) connectWs();
  }, [token, sessionsLoaded]);

  // 401 时 auth store 清空 token，这里自动回落到登录页
  if (!token) return <LoginPage />;
  if (tooNarrow) return <MinWidthGuard />;
  return <ChatLayout />;
}
