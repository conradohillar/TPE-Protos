#!/bin/bash

set -e

# === Configuration ===
PROXY="127.0.0.1:1080"
PORT=8081
FILE="100MB.bin"
CURL_FORMAT="%{time_total},%{speed_download}\n"

# === Default Settings ===
USE_CUSTOM_URL=false
ROUNDS=5
FILE_SIZE=100

# === Help Function ===
print_help() {
    echo "Usage: $0 [-h] [-r rounds] [-l url] [-s size]"
    echo "Options:"
    echo "  -h        Print this help message"
    echo "  -r N      Set number of test rounds (default: 5)"
    echo "  -l URL    Use custom URL instead of local server"
    echo "  -s SIZE   Set size of test file in MB (default: 100)"
    exit 0
}

# === Parse Arguments ===
while getopts "hr:l:s:" opt; do
    case $opt in
        h)
            print_help
            ;;
        r)
            ROUNDS="$OPTARG"
            ;;
        l)
            URL="$OPTARG"
            USE_CUSTOM_URL=true
            ;;
        s)
            FILE_SIZE="$OPTARG"
            FILE="${FILE_SIZE}MB.bin"
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            print_help
            ;;
    esac
done

if [ "$USE_CUSTOM_URL" = true ]; then
    echo "==> Using custom URL: $URL"
else
    URL="http://127.0.0.1:$PORT/$FILE"
    echo "==> No URL provided — using local server mode"
fi

# === Output logs ===
DIRECT_OUT="direct_results.csv"
PROXY_OUT="proxy_results.csv"
rm -f "$DIRECT_OUT" "$PROXY_OUT"
echo "time_total,speed_download" > "$DIRECT_OUT"
echo "time_total,speed_download" > "$PROXY_OUT"

# === Local server setup ===
if ! $USE_CUSTOM_URL; then
    if [ ! -f "$FILE" ]; then
        echo "==> Generating ${FILE_SIZE}MB test file..."
        dd if=/dev/urandom of="$FILE" bs=1M count="$FILE_SIZE" status=none
    else
        echo "==> Using existing test file: $FILE"
    fi

    echo "==> Starting local HTTP server on port $PORT..."
    python3 -m http.server "$PORT" > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1

    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "❌ Failed to start local HTTP server on port $PORT"
        exit 1
    fi

    cleanup() {
        echo "==> Cleaning up..."
        if kill -0 "$SERVER_PID" 2>/dev/null; then
            kill "$SERVER_PID"
        fi
        rm -f "$FILE"
    }
    trap cleanup EXIT
fi

#=== Run Benchmarks ===
echo "==> Running $ROUNDS rounds without proxy..."
for i in $(seq 1 "$ROUNDS"); do
    echo -n "Run $i (direct): "
    curl --fail -w "$CURL_FORMAT" -o /dev/null -s "$URL" >> "$DIRECT_OUT" || echo "Curl failed" >> "$DIRECT_OUT"
    tail -n1 "$DIRECT_OUT"
done

echo "==> Running $ROUNDS rounds with SOCKS5 proxy..."
for i in $(seq 1 "$ROUNDS"); do
    echo -n "Run $i (proxy): "
    curl --fail --socks5-hostname "$PROXY" -w "$CURL_FORMAT" -o /dev/null -s "$URL" >> "$PROXY_OUT" || echo "Curl failed" >> "$PROXY_OUT"
    tail -n1 "$PROXY_OUT"
done

# === Calculate Averages ===
calc_avg() {
    awk -F, 'NF == 2 && $2 ~ /^[0-9.]+$/ {
        total_time += $1
        speed += $2
        count += 1
    }
    END {
        if (count == 0) {
            print "No valid runs"
        } else {
            print "Avg time_total:", total_time/count, "s"
            print "Avg speed:", speed/count/1024/1024, "MB/s"
        }
    }' "$1"
}

echo
echo "==> Results (direct):"
calc_avg "$DIRECT_OUT"

echo
echo "==> Results (SOCKS5 proxy):"
calc_avg "$PROXY_OUT"
