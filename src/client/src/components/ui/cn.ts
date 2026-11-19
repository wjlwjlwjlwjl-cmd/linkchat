import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";

// 合并 className 的统一工具（AGENTS.md §2：clsx + tailwind-merge）
export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs));
}
