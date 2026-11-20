#!/usr/bin/env bash
# 一键启动全部服务
# 注意：各服务以相对路径读取 conf/settings.conf，必须在其 bin 目录下启动
# 说明：脚本可重复执行，已在运行的服务会被跳过；如需重启请先执行 ./stop.sh

set -u

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="$BASE_DIR/log"

# 运行时依赖：网关等服务的 brpc/TLS 依赖 /usr/local/lib 下的 libbotan
export LD_LIBRARY_PATH="/usr/local/lib:/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# 判断本机端口是否已被监听
is_listening() {
    (exec 3<>"/dev/tcp/127.0.0.1/$1") 2>/dev/null
}

# 从配置文件中取出服务需要监听的端口
ports_of() {
    grep -ohE '^--(server_port|gateway_port|ws_port)=[0-9]+' "$1" 2>/dev/null | cut -d= -f2
}

# 服务目录名（bin 下的目录），目录内可执行文件为 <服务名>_bin
SERVICES=(
    file_service
    gateway
    transmite_service
    friend_service
    message_storage_service
    user_service
)

mkdir -p "$LOG_DIR"

for svc in "${SERVICES[@]}"; do
    svc_dir="$BASE_DIR/bin/$svc"
    bin="$svc_dir/${svc}_bin"
    log_file="$LOG_DIR/$svc.log"
    pid_file="$LOG_DIR/$svc.pid"

    if [ ! -x "$bin" ]; then
        echo "[start] $svc: 可执行文件不存在 $bin"
        continue
    fi

    # 进程可能由手动启动或上一次 start.sh 启动，需按可执行文件判断，不能只看 pid 文件
    running_pid="$(pgrep -f "(^|/)${svc}_bin$" 2>/dev/null | head -n 1)"
    if [ -n "$running_pid" ]; then
        echo "$running_pid" > "$pid_file"
        missing=""
        for port in $(ports_of "$svc_dir/conf/settings.conf"); do
            is_listening "$port" || missing="$missing $port"
        done
        if [ -n "$missing" ]; then
            echo "[start] $svc: 进程已存在 (pid $running_pid) 但端口未监听:$missing，请查看 $log_file 或先执行 ./stop.sh"
        else
            echo "[start] $svc: 已在运行 (pid $running_pid)，跳过"
        fi
        continue
    fi

    # 必须先进入服务目录，程序用相对路径读取 conf/settings.conf
    (
        cd "$svc_dir" || exit 1
        nohup "./${svc}_bin" > "$log_file" 2>&1 &
        echo $! > "$pid_file"
    )

    sleep 0.5
    pid="$(cat "$pid_file" 2>/dev/null)"
    missing=""
    for port in $(ports_of "$svc_dir/conf/settings.conf"); do
        is_listening "$port" || missing="$missing $port"
    done
    if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null && [ -z "$missing" ]; then
        echo "[start] $svc: 启动成功 (pid $pid)"
    else
        echo "[start] $svc: 启动失败（端口未监听:$missing），请查看 $log_file"
    fi
done

# 启动前端 Vite dev server（端口 9003）
CLIENT_DIR="$BASE_DIR/src/client"
CLIENT_VITE="$CLIENT_DIR/node_modules/.bin/vite"
CLIENT_LOG="$LOG_DIR/client.log"
CLIENT_PID_FILE="$LOG_DIR/client.pid"
CLIENT_PORT=9003

if [ ! -x "$CLIENT_VITE" ]; then
    echo "[start] client: 依赖未安装，请先在 $CLIENT_DIR 执行 pnpm install"
else
    # 按 vite 进程判断，避免 pid 文件丢失时重复启动
    running_pid="$(pgrep -f "node_modules/vite/bin/vite\.js" 2>/dev/null | head -n 1)"
    if [ -n "$running_pid" ]; then
        echo "$running_pid" > "$CLIENT_PID_FILE"
        if is_listening "$CLIENT_PORT"; then
            echo "[start] client: 已在运行 (pid $running_pid)，跳过"
        else
            echo "[start] client: 进程已存在 (pid $running_pid) 但端口 $CLIENT_PORT 未监听，请查看 $CLIENT_LOG 或先执行 ./stop.sh"
        fi
    else
        (
            cd "$CLIENT_DIR" || exit 1
            nohup "$CLIENT_VITE" > "$CLIENT_LOG" 2>&1 &
            echo $! > "$CLIENT_PID_FILE"
        )

        sleep 0.5
        pid="$(cat "$CLIENT_PID_FILE" 2>/dev/null)"
        if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null && is_listening "$CLIENT_PORT"; then
            echo "[start] client: 启动成功 (pid $pid)，访问 http://localhost:$CLIENT_PORT"
        else
            echo "[start] client: 启动失败（端口 $CLIENT_PORT 未监听），请查看 $CLIENT_LOG"
        fi
    fi
fi
