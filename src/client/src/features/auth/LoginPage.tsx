import { useState } from "react";
import { nicknameLogin, getProfile } from "@/api/user";
import { useAuthStore } from "@/stores/auth";

// 深色毛玻璃登录页：品牌光晕背景 + 底边线输入框
export function LoginPage() {
  const setToken = useAuthStore((s) => s.setToken);
  const setProfile = useAuthStore((s) => s.setProfile);
  const [nickname, setNickname] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState("");
  const [submitting, setSubmitting] = useState(false);

  const canSubmit = nickname.trim().length > 0 && password.length > 0;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!canSubmit || submitting) return;
    setSubmitting(true);
    setError("");
    try {
      const rsp = await nicknameLogin({
        nickname: nickname.trim(),
        password,
      });
      if (rsp.success === true && rsp.token) {
        setToken(rsp.token);
        // 登录后立刻取当前用户资料，供气泡左右判定
        const profileRsp = await getProfile();
        if (profileRsp.success === true && profileRsp.userInfo) {
          setProfile(profileRsp.userInfo);
        }
      } else {
        setError(rsp.errmsg || "登录失败");
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "登录失败");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <div className="relative flex h-full w-full items-center justify-center overflow-hidden bg-rail">
      {/* 品牌色光晕：作为登录卡片的背景图案，从深色底中透出层次 */}
      <div
        aria-hidden="true"
        className="pointer-events-none absolute left-1/2 top-1/2 h-[800px] w-[800px] -translate-x-1/2 -translate-y-1/2 rounded-full opacity-20 blur-[120px]"
        style={{
          background:
            "radial-gradient(circle, var(--color-brand) 0%, transparent 70%)",
        }}
      />

      <form
        onSubmit={handleSubmit}
        className="relative w-[380px] rounded-xl border border-white/20 p-8 shadow-popup backdrop-blur-xl"
        style={{
          backgroundColor:
            "color-mix(in srgb, var(--bg-rail) 65%, transparent)",
        }}
      >
        <h1 className="mb-8 text-center text-2xl font-semibold text-inverse">
          LinkChat
        </h1>

        <div className="space-y-6">
          <div>
            <label
              className="mb-2 block text-sm text-secondary"
              htmlFor="nickname"
            >
              昵称
            </label>
            <input
              id="nickname"
              type="text"
              value={nickname}
              onChange={(e) => setNickname(e.target.value)}
              placeholder="请输入昵称"
              autoComplete="username"
              className="h-10 w-full rounded-none border-b border-white/20 bg-transparent px-0 text-base text-inverse transition-colors placeholder:text-secondary focus:border-brand"
            />
          </div>

          <div>
            <label
              className="mb-2 block text-sm text-secondary"
              htmlFor="password"
            >
              密码
            </label>
            <input
              id="password"
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              placeholder="请输入密码"
              autoComplete="current-password"
              className="h-10 w-full rounded-none border-b border-white/20 bg-transparent px-0 text-base text-inverse transition-colors placeholder:text-secondary focus:border-brand"
            />
          </div>

          {error && (
            <p role="alert" className="text-sm text-danger">
              {error}
            </p>
          )}

          <button
            type="submit"
            disabled={!canSubmit || submitting}
            className="h-11 w-full rounded-base bg-brand text-base font-medium text-on-brand transition-colors [@media(hover:hover)]:hover:bg-brand-hover active:bg-brand-active disabled:cursor-not-allowed disabled:bg-[var(--input-disabled-bg)] disabled:text-tertiary"
          >
            {submitting ? "登录中…" : "登录"}
          </button>
        </div>
      </form>
    </div>
  );
}
