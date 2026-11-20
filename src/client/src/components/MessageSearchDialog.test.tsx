import { beforeEach, describe, expect, it, vi } from "vitest";
import { fireEvent, render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import dayjs from "dayjs";
import { MessageSearchDialog } from "@/components/MessageSearchDialog";
import { useAuthStore } from "@/stores/auth";
import { useMessageStore } from "@/stores/message";
import { useUserStore } from "@/stores/user";
import type { ChatSessionInfo, LocalMessage, MessageInfo } from "@/types";

vi.mock("@/api/message", () => ({
  searchMessages: vi.fn(),
}));

// 图片地址 hook 会发请求，测试里直接返回 undefined
vi.mock("@/hooks/useAvatarUrl", () => ({
  useAvatarUrl: () => undefined,
}));

import { searchMessages } from "@/api/message";

const SESSION_ID = "chat_session_test";

const session: ChatSessionInfo = {
  chatSessionId: SESSION_ID,
  chatSessionName: "测试会话",
};

function buildMessage(
  id: string,
  content: string,
  createTime: number,
  senderId = "user_b",
): MessageInfo {
  return {
    messageId: id,
    chatSessionId: SESSION_ID,
    sender: { userId: senderId },
    message: { stringMessage: { content } },
    createTime: String(createTime),
  };
}

function setSnapshot(list: MessageInfo[]) {
  useMessageStore.setState({
    messages: {
      [SESSION_ID]: list.map((m) => ({ ...m, status: "sent" }) as LocalMessage),
    },
  });
}

describe("MessageSearchDialog", () => {
  beforeEach(() => {
    vi.clearAllMocks();
    useMessageStore.setState({ messages: {} });
    useAuthStore.setState({
      profile: { userId: "user_a", nickname: "我" },
    });
    useUserStore.setState({
      users: { user_b: { userId: "user_b", nickname: "对方" } },
    });
  });

  it("默认展示打开时的消息快照，按时间倒序并显示数量", () => {
    setSnapshot([
      buildMessage("msg_1", "较早的消息", 1700000000),
      buildMessage("msg_2", "较新的消息", 1700000100),
    ]);

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    expect(screen.getByText("“测试会话”的聊天记录(2)")).toBeInTheDocument();

    const contents = screen
      .getAllByText(/较早的消息|较新的消息/)
      .map((el) => el.textContent);
    expect(contents).toEqual(["较新的消息", "较早的消息"]);
    // 默认视图不调用搜索接口
    expect(searchMessages).not.toHaveBeenCalled();
  });

  it("快照为空时展示空状态", () => {
    render(<MessageSearchDialog session={session} onClose={() => {}} />);
    expect(screen.getByText("暂无聊天记录")).toBeInTheDocument();
  });

  it("输入关键词回车后调用搜索接口并展示结果", async () => {
    const user = userEvent.setup();
    setSnapshot([buildMessage("msg_1", "旧消息", 1700000000)]);
    vi.mocked(searchMessages).mockResolvedValue({
      success: true,
      msgList: [
        buildMessage("msg_9", "hello 世界", 1700000300),
        buildMessage("msg_8", "HELLO 你好", 1700000200),
      ],
    });

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    await user.type(screen.getByPlaceholderText("搜索"), "hello{Enter}");

    expect(searchMessages).toHaveBeenCalledWith({
      chatSessionId: SESSION_ID,
      searchKey: "hello",
    });
    expect(await screen.findByText("hello 世界")).toBeInTheDocument();
    expect(screen.getByText("“测试会话”的聊天记录(2)")).toBeInTheDocument();
    // 快照里的旧消息不再展示
    expect(screen.queryByText("旧消息")).not.toBeInTheDocument();
  });

  it("清空关键词回车后恢复默认快照", async () => {
    const user = userEvent.setup();
    setSnapshot([buildMessage("msg_1", "旧消息", 1700000000)]);
    vi.mocked(searchMessages).mockResolvedValue({
      success: true,
      msgList: [buildMessage("msg_9", "hello 世界", 1700000300)],
    });

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    const input = screen.getByPlaceholderText("搜索");
    await user.type(input, "hello{Enter}");
    expect(await screen.findByText("hello 世界")).toBeInTheDocument();

    await user.clear(input);
    await user.type(input, "{Enter}");

    expect(await screen.findByText("旧消息")).toBeInTheDocument();
    expect(screen.queryByText("hello 世界")).not.toBeInTheDocument();
  });

  it("搜索失败时展示后端错误信息", async () => {
    const user = userEvent.setup();
    vi.mocked(searchMessages).mockResolvedValue({
      success: false,
      errmsg: "搜索参数为空",
    });

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    await user.type(screen.getByPlaceholderText("搜索"), "hello{Enter}");

    expect(await screen.findByRole("alert")).toHaveTextContent("搜索参数为空");
    expect(screen.getByText("未找到相关聊天记录")).toBeInTheDocument();
  });

  it("接口异常时展示错误并结束加载", async () => {
    const user = userEvent.setup();
    vi.mocked(searchMessages).mockRejectedValue(new Error("网络异常"));

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    await user.type(screen.getByPlaceholderText("搜索"), "hello{Enter}");

    expect(await screen.findByRole("alert")).toHaveTextContent("网络异常");
    expect(screen.queryByText("搜索中…")).not.toBeInTheDocument();
  });

  it("按 Esc 关闭弹窗", () => {
    const onClose = vi.fn();
    render(<MessageSearchDialog session={session} onClose={onClose} />);

    fireEvent.keyDown(window, { key: "Escape" });

    expect(onClose).toHaveBeenCalledTimes(1);
  });

  it("图片消息展示缩略图，普通文件展示文件名", () => {
    setSnapshot([
      {
        messageId: "msg_img",
        chatSessionId: SESSION_ID,
        sender: { userId: "user_b" },
        message: {
          messageType: "FILE",
          fileMessage: { fileName: "photo.png", fileContent: "QUJD" },
        },
        createTime: "1700000100",
        status: "sent",
      } as LocalMessage,
      {
        messageId: "msg_file",
        chatSessionId: SESSION_ID,
        sender: { userId: "user_b" },
        message: {
          messageType: "FILE",
          fileMessage: { fileName: "report.pdf" },
        },
        createTime: "1700000200",
        status: "sent",
      } as LocalMessage,
    ]);

    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    const img = screen.getByAltText("photo.png") as HTMLImageElement;
    expect(img.src).toBe("data:image/png;base64,QUJD");
    expect(screen.getByText("[文件] report.pdf")).toBeInTheDocument();
  });

  it("展示消息时间与发送者昵称", () => {
    setSnapshot([buildMessage("msg_1", "旧消息", 1700000000)]);
    render(<MessageSearchDialog session={session} onClose={() => {}} />);

    expect(screen.getByText("对方")).toBeInTheDocument();
    expect(
      screen.getByText(dayjs.unix(1700000000).format("YYYY年M月D日 HH:mm")),
    ).toBeInTheDocument();
  });
});
