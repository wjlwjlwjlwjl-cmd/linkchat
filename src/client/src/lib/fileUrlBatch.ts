import { getMultiFile } from "@/api/file";
import { FILE_URL_TTL, useFileUrlStore } from "@/stores/fileUrl";

// 同一轮渲染内的文件请求（历史消息文件 + 各类头像）合并为一次批量预签名调用
const queue = new Set<string>();
let flushScheduled = false;

function isFresh(fileId: string): boolean {
  const hit = useFileUrlStore.getState().urls[fileId];
  return Boolean(hit && hit.expiresAt > Date.now());
}

async function flush() {
  const ids = Array.from(queue).filter((id) => !isFresh(id));
  queue.clear();
  if (ids.length === 0) return;
  try {
    const rsp = await getMultiFile(ids);
    if (rsp.success !== true || !rsp.fileData) return;
    const entries: Record<string, { url: string; expiresAt: number }> = {};
    Object.entries(rsp.fileData).forEach(([fileId, data]) => {
      if (data?.presignedUrl) {
        entries[fileId] = {
          url: data.presignedUrl,
          expiresAt: Date.now() + FILE_URL_TTL,
        };
      }
    });
    if (Object.keys(entries).length > 0) {
      useFileUrlStore.getState().setUrls(entries);
    }
  } catch {
    // 失败不阻塞渲染；后续渲染会重新入队
  }
}

// 请求某个文件的预签名地址：已缓存则直接命中，否则进入本轮的批量队列
export function requestFileUrl(fileId: string) {
  if (!fileId || isFresh(fileId)) return;
  queue.add(fileId);
  if (flushScheduled) return;
  flushScheduled = true;
  queueMicrotask(() => {
    flushScheduled = false;
    void flush();
  });
}
