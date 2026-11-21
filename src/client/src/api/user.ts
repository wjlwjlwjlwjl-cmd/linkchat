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

// 图形验证码（GET /captcha/image，返回验证码 id 与 data URL 图片）
export function getCaptcha(): Promise<
  ApiRsp & { captchaId?: string; image?: string }
> {
  return request("/captcha/image");
}

// 发送邮箱验证码（GET /login/mail_verify_code，发送前必须通过图形验证码）
export function getEmailVerifyCode(p: {
  email: string;
  captchaId: string;
  captchaCode: string;
}): Promise<ApiRsp> {
  const params = new URLSearchParams({
    email: p.email,
    captcha_id: p.captchaId,
    captcha_code: p.captchaCode,
  });
  return request(`/login/mail_verify_code?${params.toString()}`);
}

// 邮箱验证码登录（POST /login/email，body 为 proto3 JSON）
export function emailLogin(p: {
  email: string;
  verifyCode: string;
}): Promise<ApiRsp & { token?: string }> {
  return request("/login/email", {
    method: "POST",
    body: JSON.stringify({ email: p.email, verify_code: p.verifyCode }),
  });
}

// 账号注册（POST /register，邮箱验证码由 /login/mail_verify_code 获取）
export function register(p: {
  nickname: string;
  password: string;
  email: string;
  verifyCode: string;
}): Promise<ApiRsp> {
  return request("/register", {
    method: "POST",
    body: JSON.stringify({
      nickname: p.nickname,
      password: p.password,
      email: p.email,
      verify_code: p.verifyCode,
    }),
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
