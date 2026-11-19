import { useEffect, useState } from "react";
import { getSingleFile } from "@/api/file";

// 预签名 URL 有效期 15 分钟，缓存 10 分钟后重取
const TTL = 10 * 60 * 1000;

const cache = new Map<string, { url: string; expiresAt: number }>();

// 渲染期读取缓存（缓存是外部可变数据，不触发重渲染）
function readCache(fileId?: string): string {
  if (!fileId) return "";
  const hit = cache.get(fileId);
  return hit && hit.expiresAt > Date.now() ? hit.url : "";
}

// fileId（后端 avatar 字段）→ 可访问的图片 URL
export function useAvatarUrl(fileId?: string): string {
  const [resolved, setResolved] = useState<{ id?: string; url: string }>({
    url: "",
  });

  useEffect(() => {
    if (!fileId || readCache(fileId)) return;
    let cancelled = false;
    getSingleFile(fileId)
      .then((rsp) => {
        const url =
          rsp.success === true ? (rsp.fileData?.presignedUrl ?? "") : "";
        if (url) cache.set(fileId, { url, expiresAt: Date.now() + TTL });
        if (!cancelled) setResolved({ id: fileId, url });
      })
      .catch(() => {
        if (!cancelled) setResolved({ id: fileId, url: "" });
      });
    return () => {
      cancelled = true;
    };
  }, [fileId]);

  if (!fileId) return "";
  const cached = readCache(fileId);
  if (cached) return cached;
  return resolved.id === fileId ? resolved.url : "";
}
