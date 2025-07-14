#!/bin/bash

set -e

# === Configuration ===
PROXY="127.0.0.1:1080"
PORT=8081
ADMIN_PORT=8080
ADMIN_CLIENT_PATH="./bin/admin_client"
CURL_FORMAT="%{time_total},%{speed_download}\n"
RESULTS_DIR="test/benchmark_results"

# === Default Settings ===
ROUNDS=5

# Buffer sizes from 512B to 512KB (524288B)
BUFFER_SIZES=()
for i in {9..19}; do # 2^9=512, 2^19=524288
    BUFFER_SIZES+=($((2**i)))
done

# File sizes from 1MB to 128MB
FILE_SIZES=()
for i in {0..7}; do # 2^0=1, 2^7=128
    FILE_SIZES+=($((2**i)))
done

# === Help Function ===
print_help() {
    echo "Usage: $0 [-h] [-r rounds] [-a admin_port]"
    echo "Options:"
    echo "  -h        Print this help message"
    echo "  -r N      Set number of test rounds (default: 5)"
    echo "  -a PORT   Set admin server port (default: $ADMIN_PORT)"
    exit 0
}

# === Parse Arguments ===
while getopts "hr:a:" opt; do
    case $opt in
        h)
            print_help
            ;;
        r)
            ROUNDS="$OPTARG"
            ;;
        a)
            ADMIN_PORT="$OPTARG"
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            print_help
            ;;
    esac
done

# === Sanity Checks ===
if [ ! -x "$ADMIN_CLIENT_PATH" ]; then
    echo "❌ Admin client not found or not executable at '$ADMIN_CLIENT_PATH'"
    echo "Please build the project first."
    exit 1
fi

# === Setup ===
URL_BASE="http://127.0.0.1:$PORT"
SUMMARY_CSV="$RESULTS_DIR/summary.csv"

rm -rf "$RESULTS_DIR"
mkdir -p "$RESULTS_DIR"
echo "==> Results will be stored in '$SUMMARY_CSV'"

# Write header to summary CSV
echo "buffer_size_B,file_size_MB,direct_avg_time_s,direct_avg_speed_MBps,proxy_avg_time_s,proxy_avg_speed_MBps" > "$SUMMARY_CSV"

# === Local server setup ===
echo "==> Starting local HTTP server on port $PORT..."
python3 -m http.server "$PORT" > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1 # Give server time to start

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
    echo "Removing temporary test files..."
    # Suppress "No such file or directory" error if no files are found
    find . -maxdepth 1 -type f -name "*MB.bin" -delete 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# === Calculate Averages Function ===
calc_avg() {
    # Reads from stdin
    # Output format: avg_time,avg_speed_MBps
    awk -F, 'NF == 2 && $2 ~ /^[0-9.]+$/ && $1 !~ /failed/ {
        total_time += $1
        speed += $2
        count += 1
    }
    END {
        if (count == 0) {
            print "0,0" # Return 0 for failed runs
        } else {
            # printf does not add a newline, which is what we want
            printf "%.4f,%.4f", total_time/count, speed/count/1024/1024
        }
    }'
}

# === Run Benchmarks ===
for file_size in "${FILE_SIZES[@]}"; do
    FILE="${file_size}MB.bin"
    URL="$URL_BASE/$FILE"
    
    echo
    echo "========================================================================"
    echo "==> Benchmarking with file size: ${file_size}MB"
    echo "========================================================================"
    
    echo "==> Generating ${file_size}MB test file..."
    dd if=/dev/urandom of="$FILE" bs=1M count="$file_size" status=none

    for buffer_size in "${BUFFER_SIZES[@]}"; do
        echo "--- Testing with buffer size: ${buffer_size}B ---"

        if ! echo "SET_BUFF $buffer_size" | "$ADMIN_CLIENT_PATH" -p "$ADMIN_PORT" 2>/dev/null | grep -q "OK"; then
            echo "❌ Failed to set buffer size to $buffer_size. Skipping."
            # Append a line with 0s to indicate failure for this combination
            echo "$buffer_size,$file_size,0,0,0,0" >> "$SUMMARY_CSV"
            continue
        fi

        # Layered runs
        direct_run_outputs=""
        proxy_run_outputs=""

        echo -n "  Running $ROUNDS layered rounds (Direct/Proxy)... "
        for i in $(seq 1 "$ROUNDS"); do
            # Direct run
            direct_output=$(curl --fail -w "$CURL_FORMAT" -o /dev/null -s "$URL" || echo "failed,0")
            direct_run_outputs+="${direct_output}\n"

            # Proxy run
            proxy_output=$(curl --fail --socks5-hostname "$PROXY" -w "$CURL_FORMAT" -o /dev/null -s "$URL" || echo "failed,0")
            proxy_run_outputs+="${proxy_output}\n"
        done
        echo "Done."

        # Calculate averages from collected results
        direct_results=$(echo -e "$direct_run_outputs" | calc_avg)
        proxy_results=$(echo -e "$proxy_run_outputs" | calc_avg)
        
        # Append to summary CSV
        echo "$buffer_size,$file_size,$direct_results,$proxy_results" >> "$SUMMARY_CSV"
    done

    echo "==> All buffer sizes tested for ${file_size}MB file."
    rm -f "$FILE"
done

echo
echo "==> Benchmark complete."
echo "==> All results are in '$SUMMARY_CSV'."
echo "==> Final results:"
cat "$SUMMARY_CSV"

exit 0 