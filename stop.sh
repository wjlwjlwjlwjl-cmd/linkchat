#!/usr/bin/env bash
# 一键停止全部服务

set -u

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="$BASE_DIR/log"

SERVICES=(
    file_service
    gateway
    transmite_service
    friend_service
    message_storage_service
    user_service
)

stop_one() {
    local name="$1"
    local pattern="$2"
    local pid_file="$LOG_DIR/$name.pid"
    local pid=""

    if [ -f "$pid_file" ]; then
        pid="$(cat "$pid_file" 2>/dev/null)"
    fi

    # 没有 pid 文件或进程不存在时，按启动命令路径兜底查找
    if [ -z "$pid" ] || ! kill -0 "$pid" 2>/dev/null; then
        pid="$(pgrep -f "$pattern" | tr '\n' ' ')"
        if [ -z "$pid" ]; then
            echo "[stop] $name: 未运行"
            rm -f "$pid_file"
            return
        fi
    fi

    echo "[stop] $name: 正在停止 (pid $pid)"
    # shellcheck disable=SC2086
    kill $pid 2>/dev/null

    for _ in $(seq 1 20); do
        # shellcheck disable=SC2086
        if ! kill -0 $pid 2>/dev/null; then
            break
        fi
        sleep 0.2
    done

    # 超时后强制结束
    # shellcheck disable=SC2086
    kill -0 $pid 2>/dev/null && kill -9 $pid 2>/dev/null

    rm -f "$pid_file"
    echo "[stop] $name: 已停止"
}

for svc in "${SERVICES[@]}"; do
    stop_one "$svc" "(^|/)${svc}_bin\$"
done

# 停止前端 Vite dev server
stop_one "client" "node_modules/vite/bin/vite\.js\$"
