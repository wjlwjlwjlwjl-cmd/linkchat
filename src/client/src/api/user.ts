import { request, USE_MOCK } from "./client";
import {
  mockGetProfile,
  mockLogout,
  mockSetUserAvatar,
  mockSetUserDescription,
  mockSetUserNickname,
} from "./mock";
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

// 修改头像（POST /user/avatar/set，multipart 字段 avatar 为图片二进制）
export function setUserAvatar(file: Blob): Promise<ApiRsp> {
  if (USE_MOCK) return mockSetUserAvatar();
  const form = new FormData();
  form.append("avatar", file, "avatar.png");
  return request("/user/avatar/set", { method: "POST", body: form });
}

// 修改昵称（POST /user/nickname/set?nickname=，参数在 query string）
export function setUserNickname(nickname: string): Promise<ApiRsp> {
  if (USE_MOCK) return mockSetUserNickname(nickname);
  const params = new URLSearchParams({ nickname });
  return request(`/user/nickname/set?${params.toString()}`, { method: "POST" });
}

// 修改签名（POST /user/description/set?description=，空签名网关返回 400）
export function setUserDescription(description: string): Promise<ApiRsp> {
  if (USE_MOCK) return mockSetUserDescription(description);
  const params = new URLSearchParams({ description });
  return request(`/user/description/set?${params.toString()}`, {
    method: "POST",
  });
}

// 退出登录（POST /logout，token 由请求头携带，服务端删除 Redis 中的登录态）
export function logout(): Promise<ApiRsp> {
  if (USE_MOCK) return mockLogout();
  return request("/logout", { method: "POST" });
}
