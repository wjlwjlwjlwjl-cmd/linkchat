// 错误码 → UI 文案映射（AGENTS.md §6.3）
// 所有 UI 错误提示必须经此函数，禁止组件自造文案

export interface MappedError {
  message: string;
  action?: "logout" | "retry";
}

export function mapError(code: number): MappedError {
  switch (code) {
    case 400:
      return { message: "参数错误" };
    case 401:
      return { message: "登录已失效，请重新登录", action: "logout" };
    case 502:
      return { message: "服务器暂时不可用，请稍后重试", action: "retry" };
    default:
      return { message: `请求失败（${code}）` };
  }
}
