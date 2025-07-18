#!/bin/bash

set -e

# === Default Settings ===
THREADS=2
CONNECTIONS=10
DURATION="10s"
PORT=8081
FILE_SIZE=1 # Default 1MB
FILE="${FILE_SIZE}MB.bin"
URL=""

# === Help Function ===
print_help() {
    echo "Usage: $0 [-h] [-t threads] [-c connections] [-d duration] [-s size] [URL]"
    echo "Runs a concurrency benchmark using wrk and proxychains."
    echo
    echo "If no URL is provided, a local HTTP server is started to serve a test file."
    echo
    echo "Options:"
    echo "  -h        Print this help message"
    echo "  -t N      Number of threads to use (default: $THREADS)"
    echo "  -c N      Number of concurrent connections to keep open (default: $CONNECTIONS)"
    echo "  -d DUR    Duration of the test, e.g., '10s', '2m' (default: $DURATION)"
    echo "  -s SIZE   Size of local test file in MB (default: $FILE_SIZE). Only used in local server mode."
    echo
    echo "URL: The target URL to benchmark (optional)."
    echo
    echo "NOTE: This script requires 'wrk' and a proxy-chaining tool (like 'proxychains' or 'proxychains4')"
    echo "      to be installed. The proxy-chaining tool must be configured to use your SOCKS5 proxy"
    echo "      (e.g., in /etc/proxychains.conf pointing to 127.0.0.1:1080)."
    exit 0
}

# === Parse Arguments ===
while getopts "ht:c:d:s:" opt; do
    case $opt in
        h)
            print_help
            ;;
        t)
            THREADS="$OPTARG"
            ;;
        c)
            CONNECTIONS="$OPTARG"
            ;;
        d)
            DURATION="$OPTARG"
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

shift $((OPTIND-1))

USE_LOCAL_SERVER=false
if [ -z "$1" ]; then
    echo "==> No URL provided. Using local server mode."
    URL="http://127.0.0.1:$PORT/$FILE"
    USE_LOCAL_SERVER=true
else
    URL="$1"
    echo "==> Using custom URL: $URL"
fi

# === Local Server Setup ===
if [ "$USE_LOCAL_SERVER" = true ]; then
    if [ ! -f "$FILE" ]; then
        echo "==> Generating ${FILE_SIZE}MB test file..."
        dd if=/dev/urandom of="$FILE" bs=1M count="$FILE_SIZE" status=none
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
        echo -e "\n==> Cleaning up..."
        if kill -0 "$SERVER_PID" 2>/dev/null; then
            kill "$SERVER_PID"
            echo "Killed HTTP server."
        fi
        echo "Removing test file: $FILE"
        rm -f "$FILE"
    }
    trap cleanup EXIT INT TERM
fi

# === Sanity Checks ===
if ! command -v wrk &> /dev/null; then
    echo "❌ Error: 'wrk' command not found." >&2
    echo "Please install wrk to continue (e.g., 'sudo apt-get install wrk' or 'brew install wrk')." >&2
    exit 1
fi

PROXYCHAINS_CMD=""
if command -v proxychains4 &> /dev/null; then
    PROXYCHAINS_CMD="proxychains4"
elif command -v proxychains &> /dev/null; then
    PROXYCHAINS_CMD="proxychains"
else
    echo "❌ Error: Neither 'proxychains4' nor 'proxychains' command was found." >&2
    echo "Please install a proxy-chaining tool to continue (e.g., 'sudo apt-get install proxychains4')." >&2
    exit 1
fi

# === Run Benchmark ===
echo "==> Starting benchmark with the following settings:"
echo "  URL:           $URL"
echo "  Threads:       $THREADS"
echo "  Connections:   $CONNECTIONS"
echo "  Duration:      $DURATION"
echo "  Proxy Command: $PROXYCHAINS_CMD"
echo "==> All traffic will be routed through the configured proxy."
echo

"$PROXYCHAINS_CMD" wrk -t "$THREADS" -c "$CONNECTIONS" -d "$DURATION" --latency "$URL"

echo
echo "==> Benchmark complete."
