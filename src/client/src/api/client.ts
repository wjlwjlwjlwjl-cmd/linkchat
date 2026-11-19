import { mapError } from "./error";
import { useAuthStore } from "@/stores/auth";

// 仅显式 VITE_USE_MOCK=true 时走 mock，默认走真实后端（AGENTS.md §3.1）
export const USE_MOCK = import.meta.env.VITE_USE_MOCK === "true";

export const API_BASE =
  import.meta.env.VITE_API_BASE ?? "http://127.0.0.1:9000";
export const WS_BASE = import.meta.env.VITE_WS_BASE ?? "ws://127.0.0.1:9001";

// 统一请求错误：status 为 HTTP 状态码，网络故障记为 0
export class ApiError extends Error {
  status: number;

  constructor(status: number, message: string) {
    super(message);
    this.name = "ApiError";
    this.status = status;
  }
}

// 唯一 HTTP 出口（AGENTS.md §6.2）：拼 base、加 Authorization（无 Bearer）、
// 401 清 token 回登录、非 2xx 抛 ApiError（文案经 mapError）
export async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const token = useAuthStore.getState().token;
  const headers = new Headers(init?.headers);
  if (token) headers.set("Authorization", token);
  if (!headers.has("Content-Type") && init?.body) {
    headers.set("Content-Type", "application/json");
  }

  let res: Response;
  try {
    res = await fetch(`${API_BASE}${path}`, { ...init, headers });
  } catch {
    throw new ApiError(0, "网络异常，请确认后端已启动");
  }

  if (res.status === 401) {
    useAuthStore.getState().clear();
    throw new ApiError(401, mapError(401).message);
  }
  if (!res.ok) {
    throw new ApiError(res.status, mapError(res.status).message);
  }
  return (await res.json()) as T;
}

export function getToken(): string {
  return useAuthStore.getState().token;
}

export function setToken(token: string): void {
  useAuthStore.getState().setToken(token);
}

export function clearToken(): void {
  useAuthStore.getState().clear();
}
