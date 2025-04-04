#!/bin/sh

LOG_FILE="receiver_log.txt"
TARGET_IP1=172.18.0.3
TARGET_IP2=172.18.0.4
TARGET_IP3=172.18.0.5
TARGET_IP4=172.18.0.6

PORT_SEND=5555
PORT_RECV=5555

ALG=""

# Parse command-line options
if [ $# -eq 0 ]; then
echo "Usage: $0 -a <algorithm>"
exit 1
fi

while getopts "a:" opt; do
case $opt in
a) ALG=$OPTARG ;; # Capture the algorithm name
*) echo "Usage: $0 -a <algorithm>"
exit 1 ;;
esac
done

if [ -z "$ALG" ]; then
echo "Error: -a <algorithm> is mandatory"
exit 1
fi

if [ ! -f "./schedulers/${ALG}" ]; then
echo "Error: ${ALG} executable not found in ./schedulers/"
exit 1
fi

# Check for root privileges
if [ "$(id -u)" -ne 0 ]; then
echo "Please run this script with root privileges..."
exit 1
fi

# Clear previous log
> "$LOG_FILE"

echo "[$(date +%s.%N)] Receiver started on port $PORT_RECV" | tee -a "$LOG_FILE"
echo "Start signals sent to all RPis" | tee -a "$LOG_FILE"

time_start=$(date +%s.%N)

# Send STARTNOW signal to all 4 RPis using TCP
echo "STARTNOW" | ncat "$TARGET_IP1" $PORT_SEND
echo "STARTNOW" | ncat "$TARGET_IP2" $PORT_SEND
echo "STARTNOW" | ncat "$TARGET_IP3" $PORT_SEND
echo "STARTNOW" | ncat "$TARGET_IP4" $PORT_SEND

packet1=""
packet2=""
packet3=""
packet4=""

# Wait for packets from all 4 IPs
while [ -z "$packet1" ] || [ -z "$packet2" ] || [ -z "$packet3" ] || [ -z "$packet4" ]; do
packet=$(ncat -l $PORT_RECV)
sender_ip=$(echo "$packet" | grep -o "SENTIP_[0-9\.]*" | cut -d'_' -f2)

case $sender_ip in
"$TARGET_IP1") packet1="$packet" ;;
"$TARGET_IP2") packet2="$packet" ;;
"$TARGET_IP3") packet3="$packet" ;;
"$TARGET_IP4") packet4="$packet" ;;
esac
done

packets_list=""

process_packet() {
    local packet="$1"
    local sender_ip="$2"
    local packet_serial=0

    packet_sizes=$(echo "$packet" | grep -o "PACKETSIZES_[0-9 ]*" | cut -d'_' -f2-)
    for size in $packet_sizes; do
        packets_list="$packets_list SENTIP_${sender_ip}_PACKETSL_${packet_serial}_${size}"
        packet_serial=$((packet_serial + 1))
    done

    echo "[PACKET_CNT] IP_${sender_ip}_PACKET_CNT_${packet_serial}" | tee -a "$LOG_FILE"
}

process_packet "$packet1" "$TARGET_IP1"
process_packet "$packet2" "$TARGET_IP2"
process_packet "$packet3" "$TARGET_IP3"
process_packet "$packet4" "$TARGET_IP4"

reordered_packets=$(./schedulers/${ALG} $packets_list)

echo "[LOG FORMAT] [RECV] PKT_<serial>_<ip>_<size>B_<sendtime>_<time_start>_<recvtime>" | tee -a "$LOG_FILE"

for packet_entry in $reordered_packets; do
    packet_serial=$(echo "$packet_entry" | grep -o "PACKETSL_[0-9]\+" | cut -d'_' -f2)
    sender_ip=$(echo "$packet_entry" | grep -o "SENTIP_[0-9\.]*" | cut -d'_' -f2)

    time_now=$(date +%s.%N)
    send_packet_signal="SENDNOW_${packet_serial}_${time_now}_${time_start}"
    echo "$send_packet_signal" | ncat "$sender_ip" "$PORT_SEND"

    line=$(ncat -l $PORT_RECV)
    send_time=$(echo "$line" | grep -o 'SENTTIME_[0-9\.]*' | cut -d'_' -f2)
    recv_time_start=$(echo "$line" | grep -o 'TIMESTART_[0-9\.]*' | cut -d'_' -f2)
    recv_time=$(date +%s.%N)
    size=${#line}
    packet_serial=$(echo "$line" | grep -o "PKT_[0-9]\+_" | cut -d'_' -f2)
    sender_ip=$(echo "$line" | grep -o "SENTIP_[0-9\.]*" | cut -d'_' -f2)

    echo "[RECV] PKT_${packet_serial}_SENTIP_${sender_ip}_SIZE_${size}B_SENDTIME_${send_time}_TIMESTART_${recv_time_start}_RECVTIME_${recv_time}" | tee -a "$LOG_FILE"
done

for target_ip in "$TARGET_IP1" "$TARGET_IP2" "$TARGET_IP3" "$TARGET_IP4"; do
echo "DONESEND" | ncat "$target_ip" "$PORT_SEND"
done

echo "All packets received..." | tee -a "$LOG_FILE"

./statistics

echo "Statistics script completed" | tee -a "$LOG_FILE"