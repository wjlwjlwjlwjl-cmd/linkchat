#!/bin/bash

HOST="http://127.0.0.1:9000"
SERVICE_NAME="FileService"
METHOD_NAME="GetSingleFile"
FILE_ID="file_cc138cb4.jpg" 
echo "正在获取 $FILE_ID 的预签名 URL..."

# 发送请求
RESP=$(curl -s -X POST "${HOST}/${SERVICE_NAME}/${METHOD_NAME}" \
     -H "Content-Type: application/json" \
     -d "{
           \"request_id\": \"req-get-url-001\",
           \"file_id\": \"${FILE_ID}\"
         }")

# 检查是否成功
if [[ "$RESP" == *"\"success\":true"* ]]; then
    # 提取 presigned_url 的值
    # 原理：匹配 "presigned_url":" 之后，直到下一个双引号之前的所有字符
    URL=$(echo "$RESP" | grep -o '"presigned_url":"[^"]*"' | cut -d'"' -f4)
    
    if [ -n "$URL" ]; then
        echo "获取成功！"
        echo "预签名 URL: $URL"
    else
        echo "解析 URL 失败，原始响应如下："
        echo "$RESP"
    fi
else
    echo "获取失败，服务端原始响应如下："
    echo "$RESP"
fi