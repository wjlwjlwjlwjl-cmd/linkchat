import { useEffect, useState } from "react";
import { ChatLayout } from "@/features/chat/ChatLayout";
import { LoginPage } from "@/features/auth/LoginPage";
import { MinWidthGuard } from "@/components/MinWidthGuard";
import { useAuthStore } from "@/stores/auth";
import { connectWs, disconnectWs } from "@/ws/client";

// 根组件：登录守卫 → 窗口宽度守卫 → 三栏布局
export default function App() {
  const token = useAuthStore((s) => s.token);
  const [tooNarrow, setTooNarrow] = useState(
    typeof window !== "undefined" ? window.innerWidth < 1024 : false,
  );

  useEffect(() => {
    const onResize = () => setTooNarrow(window.innerWidth < 1024);
    window.addEventListener("resize", onResize);
    return () => window.removeEventListener("resize", onResize);
  }, []);

  // 登录后建立 WS，token 清空（退出 / 401）时断开且不再重连
  useEffect(() => {
    if (token) connectWs();
    else disconnectWs();
  }, [token]);

  // 401 时 auth store 清空 token，这里自动回落到登录页
  if (!token) return <LoginPage />;
  if (tooNarrow) return <MinWidthGuard />;
  return <ChatLayout />;
}
