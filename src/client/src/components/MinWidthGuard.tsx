import { MonitorSmartphone } from "lucide-react";

// 窗口宽度 <1024px 时显示提示页，不做移动端适配（AGENTS.md §4.9 / style.md §6）
export function MinWidthGuard() {
  return (
    <div className="flex h-full w-full flex-col items-center justify-center gap-4 bg-app">
      <MonitorSmartphone
        size={64}
        strokeWidth={1.5}
        style={{ color: "var(--empty-icon)" }}
      />
      <h2 className="text-xl text-secondary">请放大窗口</h2>
      <p className="text-sm text-tertiary">
        LinkChat 仅支持桌面端，请将窗口放大至 1024×640 以上
      </p>
    </div>
  );
}
