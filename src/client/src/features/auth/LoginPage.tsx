import { useState } from "react";
import { nicknameLogin, getProfile } from "@/api/user";
import { useAuthStore } from "@/stores/auth";

// 极简登录页：昵称 + 密码（POST /login/nickname）
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
    <div className="flex h-full w-full items-center justify-center bg-app">
      <form
        onSubmit={handleSubmit}
        className="w-[360px] rounded-lg border border-line bg-panel p-8"
      >
        <h1 className="mb-6 text-center text-xl font-medium text-primary">
          LinkChat
        </h1>

        <label className="mb-1 block text-sm text-secondary" htmlFor="nickname">
          昵称
        </label>
        <input
          id="nickname"
          type="text"
          value={nickname}
          onChange={(e) => setNickname(e.target.value)}
          placeholder="请输入昵称"
          autoComplete="username"
          className="mb-4 h-9 w-full rounded-base border border-line bg-input px-3 text-sm text-primary placeholder:text-tertiary"
        />

        <label className="mb-1 block text-sm text-secondary" htmlFor="password">
          密码
        </label>
        <input
          id="password"
          type="password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          placeholder="请输入密码"
          autoComplete="current-password"
          className="mb-4 h-9 w-full rounded-base border border-line bg-input px-3 text-sm text-primary placeholder:text-tertiary"
        />

        {error && (
          <p
            role="alert"
            className="mb-3 text-sm"
            style={{ color: "var(--color-danger)" }}
          >
            {error}
          </p>
        )}

        <button
          type="submit"
          disabled={!canSubmit || submitting}
          className="h-9 w-full rounded-base text-sm disabled:cursor-not-allowed"
          style={{
            background:
              canSubmit && !submitting
                ? "var(--color-brand)"
                : "var(--input-disabled-bg)",
            color:
              canSubmit && !submitting
                ? "var(--text-on-brand)"
                : "var(--text-tertiary)",
          }}
        >
          {submitting ? "登录中…" : "登录"}
        </button>
      </form>
    </div>
  );
}
