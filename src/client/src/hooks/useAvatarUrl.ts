import { useEffect } from "react";
import { requestFileUrl } from "@/lib/fileUrlBatch";
import { useFileUrlStore } from "@/stores/fileUrl";

// fileId（后端 avatar 字段）→ 可访问的图片 URL
// 地址统一由批量预签名接口填充，这里只负责订阅与入队
export function useAvatarUrl(fileId?: string): string {
  const url = useFileUrlStore((s) =>
    fileId ? (s.urls[fileId]?.url ?? "") : "",
  );

  useEffect(() => {
    if (fileId && !url) requestFileUrl(fileId);
  }, [fileId, url]);

  return url;
}
