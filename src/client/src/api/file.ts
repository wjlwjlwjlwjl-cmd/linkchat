import { request, USE_MOCK } from "./client";
import type { ApiRsp, FileDownloadData } from "@/types";

// 获取单个文件的预签名下载地址（GET /file/get_single，URL 15 分钟过期）
export function getSingleFile(
  fileId: string,
): Promise<ApiRsp & { fileData?: FileDownloadData }> {
  if (USE_MOCK) return Promise.resolve({ success: true });
  const params = new URLSearchParams({ file_id: fileId });
  return request(`/file/get_single?${params.toString()}`);
}
