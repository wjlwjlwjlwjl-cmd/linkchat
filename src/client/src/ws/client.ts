import { getRecent } from "@/api/message";
import { useAuthStore } from "@/stores/auth";
import { useMessageStore } from "@/stores/message";
import { useSessionStore } from "@/stores/session";
import { dispatch } from "./dispatcher";

// WS 连接管理（AGENTS.md §6.8）：30s 心跳 + 指数退避重连 + 重连补齐
const HEARTBEAT_MS = 30_000;
const MAX_RECONNECT_MS = 30_000;

let socket: WebSocket | null = null;
let heartbeatTimer: number | undefined;
let reconnectTimer: number | undefined;
let attempt = 0;
let manualClose = false;

function wsUrl(): string {
  const base = import.meta.env.VITE_WS_BASE || "ws://127.0.0.1:9001";
  const token = useAuthStore.getState().token ?? "";
  return `${base}/?token=${encodeURIComponent(token)}`;
}

function startHeartbeat(): void {
  stopHeartbeat();
  heartbeatTimer = window.setInterval(() => {
    // 网关忽略客户端消息，这里只用于保活
    if (socket?.readyState === WebSocket.OPEN) socket.send("ping");
  }, HEARTBEAT_MS);
}

function stopHeartbeat(): void {
  if (heartbeatTimer !== undefined) {
    window.clearInterval(heartbeatTimer);
    heartbeatTimer = undefined;
  }
}

// 重连/首连成功：拉当前会话最近消息，按 messageId 去重补齐
async function backfillRecent(): Promise<void> {
  const sessionId = useSessionStore.getState().activeSessionId;
  if (!sessionId) return;
  try {
    const rsp = await getRecent({ chatSessionId: sessionId, msgCount: 50 });
    if (rsp.success === true) {
      useMessageStore.getState().dedupeById(sessionId, rsp.msgList ?? []);
    }
  } catch {
    // 补齐失败不影响连接，下次重连再试
  }
}

function scheduleReconnect(): void {
  if (manualClose || !useAuthStore.getState().token) return;
  const delay = Math.min(1000 * 2 ** attempt, MAX_RECONNECT_MS);
  attempt += 1;
  reconnectTimer = window.setTimeout(open, delay);
}

function open(): void {
  if (!useAuthStore.getState().token) return;
  const ws = new WebSocket(wsUrl());
  socket = ws;

  ws.onopen = () => {
    attempt = 0;
    startHeartbeat();
    void backfillRecent();
  };
  ws.onmessage = (e: MessageEvent) => {
    if (typeof e.data === "string") dispatch(e.data);
  };
  ws.onerror = () => {
    ws.close();
  };
  ws.onclose = () => {
    stopHeartbeat();
    if (socket === ws) socket = null;
    scheduleReconnect();
  };
}

// 登录后建立连接（幂等：已连接/连接中不重复建）
export function connectWs(): void {
  if (!useAuthStore.getState().token) return;
  manualClose = false;
  if (
    socket &&
    (socket.readyState === WebSocket.OPEN ||
      socket.readyState === WebSocket.CONNECTING)
  ) {
    return;
  }
  open();
}

// 退出登录 / 401 时断开，不再重连
export function disconnectWs(): void {
  manualClose = true;
  if (reconnectTimer !== undefined) {
    window.clearTimeout(reconnectTimer);
    reconnectTimer = undefined;
  }
  stopHeartbeat();
  const cur = socket;
  socket = null;
  attempt = 0;
  cur?.close();
}
