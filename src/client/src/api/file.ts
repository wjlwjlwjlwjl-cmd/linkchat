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

// 批量获取预签名下载地址（GET /file/get_multi，file_ids 重复传参）
// 返回的 fileData 是 file_id → FileDownloadData 的映射
export function getMultiFile(
  fileIds: string[],
): Promise<ApiRsp & { fileData?: Record<string, FileDownloadData> }> {
  if (USE_MOCK) return Promise.resolve({ success: true });
  const params = new URLSearchParams();
  fileIds.forEach((id) => params.append("file_ids", id));
  return request(`/file/get_multi?${params.toString()}`);
}
