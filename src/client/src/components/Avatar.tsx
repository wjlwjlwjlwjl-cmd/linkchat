import { useState } from "react";
import { useAvatarUrl } from "@/hooks/useAvatarUrl";

interface AvatarProps {
  fileId?: string; // 后端 avatar 字段（file_id）
  name: string; // 无头像时取首字
  size?: number; // 边长 px，默认 40
}

// 统一头像：有预签名 URL 显示图片，否则/加载失败回退昵称首字
export function Avatar({ fileId, name, size = 40 }: AvatarProps) {
  const url = useAvatarUrl(fileId);
  const [broken, setBroken] = useState(false);
  const letter = name.trim().slice(0, 1) || "?";

  return (
    <div
      className="flex shrink-0 items-center justify-center overflow-hidden bg-brand-soft text-brand"
      style={{
        width: size,
        height: size,
        borderRadius: "var(--avatar-radius)",
      }}
    >
      {url && !broken ? (
        <img
          src={url}
          alt={name}
          className="h-full w-full object-cover"
          onError={() => setBroken(true)}
        />
      ) : (
        <span style={{ fontSize: Math.round(size * 0.35) }}>{letter}</span>
      )}
    </div>
  );
}
