import { request, USE_MOCK } from "./client";
import { mockGetProfile } from "./mock";
import type { ApiRsp, UserInfo } from "@/types";

// 昵称密码登录（POST /login/nickname，白名单接口）
export function nicknameLogin(p: {
  nickname: string;
  password: string;
}): Promise<ApiRsp & { token?: string }> {
  return request("/login/nickname", {
    method: "POST",
    body: JSON.stringify(p),
  });
}

// 获取当前登录用户信息（GET /user/info，无参数，user_id 从 token 解析）
export function getProfile(): Promise<ApiRsp & { userInfo: UserInfo }> {
  if (USE_MOCK) return mockGetProfile();
  return request("/user/info");
}

// 批量获取用户信息（GET /user/multi_info?users_id=A&users_id=B）
// 用于补全消息 sender 的昵称 / 头像（消息里 sender 只有 userId）
export function getMultiUserInfo(
  userIds: string[],
): Promise<ApiRsp & { usersInfo?: Record<string, UserInfo> }> {
  const params = new URLSearchParams();
  userIds.forEach((id) => params.append("users_id", id));
  return request(`/user/multi_info?${params.toString()}`);
}
