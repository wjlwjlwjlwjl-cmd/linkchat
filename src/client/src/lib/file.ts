// 图片消息没有单独的 MIME 字段，按文件名后缀推导
const IMAGE_MIME: Record<string, string> = {
  png: "image/png",
  jpg: "image/jpeg",
  jpeg: "image/jpeg",
  gif: "image/gif",
  webp: "image/webp",
  bmp: "image/bmp",
};

function extOf(fileName?: string): string {
  const name = fileName ?? "";
  const dot = name.lastIndexOf(".");
  return dot >= 0 ? name.slice(dot + 1).toLowerCase() : "";
}

export function imageMimeType(fileName?: string): string | undefined {
  return IMAGE_MIME[extOf(fileName)];
}

export function isImageFileName(fileName?: string): boolean {
  return imageMimeType(fileName) !== undefined;
}

// 文件大小文案：8B / 8.3K / 1.2M
export function formatFileSize(bytes: number): string {
  if (!Number.isFinite(bytes) || bytes <= 0) return "";
  if (bytes < 1024) return `${bytes}B`;
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)}K`;
  return `${(bytes / 1024 / 1024).toFixed(1)}M`;
}
