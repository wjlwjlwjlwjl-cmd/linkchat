#!/bin/bash

HOST="http://127.0.0.1:9000"
# 改成 proto 里定义的 service 名字，注意大小写
SERVICE_NAME="FileService" 
METHOD_NAME="PutSingleFile"
IMAGE_FILE="test.jpg"
TMP_JSON="payload.json"

if [ ! -f "$IMAGE_FILE" ]; then
    echo "错误: 找不到 $IMAGE_FILE"
    exit 1
fi

FILE_SIZE=$(wc -c < "$IMAGE_FILE" | tr -d ' ')
FILE_CONTENT_BASE64=$(base64 -w 0 "$IMAGE_FILE") 

cat <<EOF > "$TMP_JSON"
{
  "request_id": "req-upload-001",
  "file_data": {
    "file_name": "$IMAGE_FILE",
    "file_size": $FILE_SIZE,
    "file_content": "$FILE_CONTENT_BASE64"
  }
}
EOF

echo "正在上传 $IMAGE_FILE (大小: $FILE_SIZE bytes)..."

curl -X POST "${HOST}/${SERVICE_NAME}/${METHOD_NAME}" \
     -H "Content-Type: application/json" \
     -d @"$TMP_JSON"

rm -f "$TMP_JSON"
echo ""