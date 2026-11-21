import { useEffect, useState } from "react";
import {
  emailLogin,
  getCaptcha,
  getEmailVerifyCode,
  getProfile,
  nicknameLogin,
  register as registerAccount,
} from "@/api/user";
import { useAuthStore } from "@/stores/auth";

type Mode = "password" | "email" | "register";

const EMAIL_RE = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;

const TABS: { key: Mode; label: string }[] = [
  { key: "password", label: "密码登录" },
  { key: "email", label: "邮箱登录" },
  { key: "register", label: "注册" },
];

// 常规输入框：仅保留底边线，聚焦时底边变微信绿
const inputClass =
  "h-10 w-full rounded-none border-b border-[var(--login-border)] bg-transparent px-0 text-[15px] text-[var(--login-title)] transition-colors placeholder:text-[var(--login-placeholder)] focus:border-[var(--login-primary)] focus:outline-none focus-visible:outline-none";

// 图形验证码输入框：置于浅灰底块内，无边框
const captchaInputClass =
  "h-9 min-w-0 flex-1 bg-transparent px-1 text-[15px] text-[var(--login-title)] placeholder:text-[var(--login-placeholder)] focus:outline-none focus-visible:outline-none";

// 登录页 Logo：微信绿圆角方块 + 白色对话气泡
function Logo() {
  return (
    <svg
      width="56"
      height="56"
      viewBox="0 0 56 56"
      role="img"
      aria-label="LinkChat"
    >
      <rect width="56" height="56" rx="14" fill="var(--login-primary)" />
      <path
        d="M28 13c-8.7 0-15.8 5.8-15.8 13 0 4.1 2.4 7.8 6 10.2L16.7 42l6.3-3.1c1.6.4 3.3.6 5 .6 8.7 0 15.8-5.8 15.8-13.5S36.7 13 28 13Z"
        fill="#ffffff"
      />
      <circle cx="21.5" cy="26" r="2.2" fill="var(--login-primary)" />
      <circle cx="28" cy="26" r="2.2" fill="var(--login-primary)" />
      <circle cx="34.5" cy="26" r="2.2" fill="var(--login-primary)" />
    </svg>
  );
}

export function LoginPage() {
  const setToken = useAuthStore((s) => s.setToken);
  const setProfile = useAuthStore((s) => s.setProfile);

  const [mode, setMode] = useState<Mode>("password");
  const [nickname, setNickname] = useState("");
  const [password, setPassword] = useState("");
  const [email, setEmail] = useState("");
  const [code, setCode] = useState("");
  const [error, setError] = useState("");
  const [notice, setNotice] = useState("");
  const [submitting, setSubmitting] = useState(false);

  // 图形验证码：点击“获取验证码”后才展示，确认通过后才发送邮箱验证码
  const [captcha, setCaptcha] = useState<{
    id: string;
    image: string;
  } | null>(null);
  const [captchaCode, setCaptchaCode] = useState("");
  const [sendingCode, setSendingCode] = useState(false);
  const [cooldown, setCooldown] = useState(0);

  useEffect(() => {
    if (cooldown <= 0) return;
    const timer = window.setTimeout(() => setCooldown((c) => c - 1), 1000);
    return () => window.clearTimeout(timer);
  }, [cooldown]);

  const switchMode = (next: Mode) => {
    setMode(next);
    setError("");
    setNotice("");
    setCaptcha(null);
    setCaptchaCode("");
  };

  const loadCaptcha = async () => {
    try {
      const rsp = await getCaptcha();
      if (rsp.success === true && rsp.captchaId && rsp.image) {
        setCaptcha({ id: rsp.captchaId, image: rsp.image });
        setCaptchaCode("");
      } else {
        setError(rsp.errmsg || "图形验证码加载失败");
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "图形验证码加载失败");
    }
  };

  // 点击“获取验证码”：先校验邮箱，再拉取图形验证码
  const handleRequestCode = () => {
    setError("");
    setNotice("");
    if (!EMAIL_RE.test(email.trim())) {
      setError("请输入正确的邮箱地址");
      return;
    }
    void loadCaptcha();
  };

  const handleRefreshCaptcha = () => {
    setError("");
    void loadCaptcha();
  };

  // 图形验证码通过后才请求发送邮箱验证码
  const handleConfirmCaptcha = async () => {
    if (!captcha || sendingCode) return;
    if (!captchaCode.trim()) {
      setError("请输入图形验证码");
      return;
    }
    setSendingCode(true);
    setError("");
    try {
      const rsp = await getEmailVerifyCode({
        email: email.trim(),
        captchaId: captcha.id,
        captchaCode: captchaCode.trim(),
      });
      if (rsp.success === true) {
        setCaptcha(null);
        setCaptchaCode("");
        setNotice("验证码已发送，请查收邮箱");
        setCooldown(60);
      } else {
        setError(rsp.errmsg || "发送验证码失败");
        // 图形验证码一次性使用，失败后换一张
        void loadCaptcha();
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "发送验证码失败");
      void loadCaptcha();
    } finally {
      setSendingCode(false);
    }
  };

  const finishLogin = async (token: string) => {
    setToken(token);
    // 登录后立刻取当前用户资料，供气泡左右判定
    const profileRsp = await getProfile();
    if (profileRsp.success === true && profileRsp.userInfo) {
      setProfile(profileRsp.userInfo);
    }
  };

  const canSubmit =
    mode === "password"
      ? nickname.trim().length > 0 && password.length > 0
      : mode === "email"
        ? EMAIL_RE.test(email.trim()) && code.trim().length > 0
        : nickname.trim().length > 0 &&
          EMAIL_RE.test(email.trim()) &&
          code.trim().length > 0 &&
          password.length > 0;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!canSubmit || submitting) return;
    setSubmitting(true);
    setError("");
    setNotice("");
    try {
      if (mode === "password") {
        const rsp = await nicknameLogin({
          nickname: nickname.trim(),
          password,
        });
        if (rsp.success === true && rsp.token) {
          await finishLogin(rsp.token);
        } else {
          setError(rsp.errmsg || "登录失败");
        }
      } else if (mode === "email") {
        const rsp = await emailLogin({
          email: email.trim(),
          verifyCode: code.trim(),
        });
        if (rsp.success === true && rsp.token) {
          await finishLogin(rsp.token);
        } else {
          setError(rsp.errmsg || "登录失败");
        }
      } else {
        const rsp = await registerAccount({
          nickname: nickname.trim(),
          password,
          email: email.trim(),
          verifyCode: code.trim(),
        });
        if (rsp.success === true) {
          switchMode("password");
          setPassword("");
          setCode("");
          setNotice("注册成功，请使用密码登录");
        } else {
          setError(rsp.errmsg || "注册失败");
        }
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "操作失败");
    } finally {
      setSubmitting(false);
    }
  };

  const showCodeFields = mode === "email" || mode === "register";

  return (
    <div className="h-full w-full overflow-y-auto overflow-x-hidden bg-[var(--login-bg)]">
      {/* 内容高于视口时整体可滚动，避免图形验证码展开后字段被裁切 */}
      <div className="flex min-h-full items-center justify-center px-4 py-10">
        <form
          onSubmit={handleSubmit}
          className="login-card w-full max-w-[400px] rounded-[8px] bg-[var(--login-card)] p-10"
        >
          <div className="mb-4 flex justify-center">
            <Logo />
          </div>
          <h1 className="mb-10 text-center text-[22px] font-semibold tracking-wide text-[var(--login-title)]">
            LinkChat
          </h1>

          {/* 纯文字 Tab：选中加粗变黑，底部一条微信绿短横线 */}
          <div className="mb-10 flex items-center justify-center gap-12">
            {TABS.map((tab) => (
              <button
                key={tab.key}
                type="button"
                onClick={() => switchMode(tab.key)}
                className={`relative pb-2 text-[15px] transition-colors ${
                  mode === tab.key
                    ? "font-semibold text-[var(--login-title)]"
                    : "text-[var(--login-placeholder)] [@media(hover:hover)]:hover:text-[var(--text-secondary)]"
                }`}
              >
                {tab.label}
                {mode === tab.key && (
                  <span className="absolute bottom-0 left-1/2 h-[3px] w-5 -translate-x-1/2 rounded-full bg-[var(--login-primary)]" />
                )}
              </button>
            ))}
          </div>

          <div className="space-y-6">
            {mode !== "email" && (
              <div>
                <label
                  className="mb-2 block text-[13px] text-[var(--text-secondary)]"
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
                  className={inputClass}
                />
              </div>
            )}

            {mode === "password" && (
              <div>
                <label
                  className="mb-2 block text-[13px] text-[var(--text-secondary)]"
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
                  className={inputClass}
                />
              </div>
            )}

            {showCodeFields && (
              <>
                <div>
                  <label
                    className="mb-2 block text-[13px] text-[var(--text-secondary)]"
                    htmlFor="email"
                  >
                    邮箱
                  </label>
                  <input
                    id="email"
                    type="email"
                    value={email}
                    onChange={(e) => setEmail(e.target.value)}
                    placeholder="请输入邮箱"
                    autoComplete="email"
                    className={inputClass}
                  />
                </div>

                <div>
                  <label
                    className="mb-2 block text-[13px] text-[var(--text-secondary)]"
                    htmlFor="code"
                  >
                    邮箱验证码
                  </label>
                  <div className="flex items-end gap-3">
                    <input
                      id="code"
                      type="text"
                      inputMode="numeric"
                      maxLength={6}
                      value={code}
                      onChange={(e) => setCode(e.target.value)}
                      placeholder="请输入验证码"
                      autoComplete="one-time-code"
                      className={inputClass}
                    />
                    <button
                      type="button"
                      onClick={handleRequestCode}
                      disabled={cooldown > 0 || sendingCode}
                      className="h-10 shrink-0 rounded-[4px] border border-[var(--login-primary)] px-3 text-[13px] text-[var(--login-primary)] transition-colors disabled:cursor-not-allowed disabled:border-[var(--login-border)] disabled:text-[var(--login-placeholder)] [@media(hover:hover)]:hover:bg-[var(--login-primary-soft)]"
                    >
                      {cooldown > 0 ? `${cooldown}s 后重发` : "获取验证码"}
                    </button>
                  </div>
                </div>
              </>
            )}

            {mode === "register" && (
              <div>
                <label
                  className="mb-2 block text-[13px] text-[var(--text-secondary)]"
                  htmlFor="register-password"
                >
                  密码
                </label>
                <input
                  id="register-password"
                  type="password"
                  value={password}
                  onChange={(e) => setPassword(e.target.value)}
                  placeholder="请设置密码"
                  autoComplete="new-password"
                  className={inputClass}
                />
              </div>
            )}

            {captcha && (
              <div className="rounded-[6px] bg-[var(--login-input-bg)] p-3">
                <p className="mb-2 text-[12px] text-[var(--text-secondary)]">
                  请输入图形验证码后发送邮箱验证码
                </p>
                <div className="flex items-center gap-2">
                  <img
                    src={captcha.image}
                    alt="图形验证码"
                    title="点击刷新"
                    onClick={handleRefreshCaptcha}
                    className="h-10 w-[110px] shrink-0 cursor-pointer select-none rounded-[4px] bg-[var(--login-card)]"
                  />
                  <input
                    type="text"
                    maxLength={4}
                    value={captchaCode}
                    onChange={(e) => setCaptchaCode(e.target.value)}
                    onKeyDown={(e) => {
                      if (e.key === "Enter") {
                        e.preventDefault();
                        void handleConfirmCaptcha();
                      }
                    }}
                    placeholder="图形验证码"
                    autoComplete="off"
                    className={captchaInputClass}
                  />
                  <button
                    type="button"
                    onClick={handleRefreshCaptcha}
                    className="shrink-0 text-[13px] text-[var(--login-link)] [@media(hover:hover)]:hover:underline"
                  >
                    换一张
                  </button>
                </div>
                <button
                  type="button"
                  onClick={() => void handleConfirmCaptcha()}
                  disabled={sendingCode}
                  className="mt-3 h-9 w-full rounded-[4px] bg-[var(--login-primary)] text-[14px] text-[var(--text-inverse)] transition-colors [@media(hover:hover)]:hover:bg-[var(--login-primary-hover)] active:bg-[var(--login-primary-active)] disabled:cursor-not-allowed disabled:bg-[var(--login-border)]"
                >
                  {sendingCode ? "发送中…" : "发送验证码"}
                </button>
              </div>
            )}

            {error && (
              <p
                role="alert"
                className="text-[13px] text-[var(--color-danger)]"
              >
                {error}
              </p>
            )}
            {notice && (
              <p
                role="status"
                className="text-[13px] text-[var(--login-primary)]"
              >
                {notice}
              </p>
            )}

            <button
              type="submit"
              disabled={!canSubmit || submitting}
              className="h-11 w-full rounded-[4px] bg-[var(--login-primary)] text-[15px] font-medium text-[var(--text-inverse)] transition-colors [@media(hover:hover)]:hover:bg-[var(--login-primary-hover)] active:bg-[var(--login-primary-active)] disabled:cursor-not-allowed disabled:bg-[var(--login-border)] disabled:text-[var(--login-placeholder)]"
            >
              {submitting
                ? mode === "register"
                  ? "注册中…"
                  : "登录中…"
                : mode === "register"
                  ? "注册"
                  : "登录"}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
