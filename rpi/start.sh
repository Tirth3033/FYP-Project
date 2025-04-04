#!/bin/bash

MAX_SIZE=-1
MIN_SIZE=-1
NUM_PACKETS=-1

TARGET_IP=172.18.0.2
LOG_FILE="sender_log.txt"
MAX_SIZE_DEFAULT=10024
MIN_SIZE_DEFAULT=64
NUM_PACKETS_DEFAULT=100
PORT_SEND=5555
PORT_RECV=5555

# Parse command-line options
while getopts "M:m:n:" opt; do
  case $opt in
    M)
      case $OPTARG in
        ''|*[!0-9]*)
          echo "Error: -M must be followed by a number meaning the maximum packet size in bytes"
          exit 1
          ;;
        *)
          MAX_SIZE=$OPTARG
          echo "Maximum packet size set to $MAX_SIZE"
          ;;
      esac
      ;;
    m)
      case $OPTARG in
        ''|*[!0-9]*)
          echo "Error: -m must be followed by a number meaning the minimum packet size in bytes"
          exit 1
          ;;
        *)
          MIN_SIZE=$OPTARG
          echo "Minimum packet size set to $MIN_SIZE"
          ;;
      esac
      ;;
    n)
      case $OPTARG in
        ''|*[!0-9]*)
          echo "Error: -n must be followed by a number meaning the number of packets to send"
          exit 1
          ;;
        *)
          NUM_PACKETS=$OPTARG
          echo "Number of packets set to $NUM_PACKETS"
          ;;
      esac
      ;;
    *) echo "Usage: $0 -M <max packet size>(optional) -m <min packet size>(optional) -n <number of packets>(optional)"
       exit 1 ;;
  esac
done

# Check if the script is run with root privileges
if [ "$(id -u)" -ne 0 ]; then
    echo "Please run this script with root privileges..."
    exit 1
fi

# Set default values from config.txt if not specified
if [ "$MAX_SIZE" = "-1" ]; then
    MAX_SIZE=$MAX_SIZE_DEFAULT
    echo "Using default maximum packet size: $MAX_SIZE"
fi

if [ "$MIN_SIZE" = "-1" ]; then
    MIN_SIZE=$MIN_SIZE_DEFAULT
    echo "Using default minimum packet size: $MIN_SIZE"
fi

if [ "$NUM_PACKETS" = "-1" ]; then
    NUM_PACKETS=$NUM_PACKETS_DEFAULT
    echo "Using default number of packets: $NUM_PACKETS"
fi

# Clear previous log
: > "$LOG_FILE"

# Wait for start signal on port 5555 using TCP
echo "Waiting for start signal on port $PORT_RECV..." | tee -a "$LOG_FILE"

ncat -l -p $PORT_RECV > temp_file
signal=$(cat temp_file)
hostname -I | awk '{print $1}' > temp_file
ip=$(cat temp_file)
rm temp_file

if [ "$signal" = "STARTNOW" ]; then
    echo "Received start signal, beginning transmission..." | tee -a "$LOG_FILE"
else
    echo "No start signal received, exiting." | tee -a "$LOG_FILE"
    exit 1
fi

# Create a list of random packet sizes and packets
packets=()
packet_sizes=()
for ((i=0; i<NUM_PACKETS; i++)); do
    size=$((RANDOM % (MAX_SIZE - MIN_SIZE + 1) + MIN_SIZE))
    packet_sizes+=($size)
    packet="PKT_${packet_serial}_SENTIP_${ip}_$(head -c "$size" /dev/zero | tr '\0' 'A')"
    packet_size=${#packet}
    packet="PKT_${packet_serial}_SENTIP_${ip}_${packet_size}B_SENTTIME_${sent_time}_$(head -c "$size" /dev/zero | tr '\0' 'A')"
    packets+=("$packet")
done
# Create a packet with this size array and also the ip of this device
packet="ARRAYPACKET_SENTIP_${ip}_PACKETSIZES_${packet_sizes[@]}"
# Send the packet
while ! echo "$packet" | ncat "$TARGET_IP" "$PORT_SEND"; do
    echo "Port $PORT_SEND is occupied, retrying..." | tee -a "$LOG_FILE"
    sleep 1
done

# PKT_52_172.18.0.3_9890B_1743332379.172969888
echo "[LOG FORMAT] [SEND] PKT_<serial>_<ip>_<size>B_<senttime>_<time_start>" | tee -a "$LOG_FILE"

# Function to send a single packet
send_packet() {
    local index=$1
    local size=${packet_sizes[$index]}
    local sent_time=$2 # $(date +%s.%N)
    local time_start=$3
    local packet_serial=$index # Packet serial number

    local packet="PKT_${packet_serial}_SENTIP_${ip}_$(head -c "$size" /dev/zero | tr '\0' 'A')"
    local packet_size=${#packet}
    packet="PKT_${packet_serial}_SENTIP_${ip}_${packet_size}B_SENTTIME_${sent_time}_TIMESTART_${time_start}_$(head -c "$size" /dev/zero | tr '\0' 'A')"

    echo "[SEND] PKT_${packet_serial}_${ip}_${packet_size}B_${sent_time}_${time_start}" | tee -a "$LOG_FILE" >&2

    # Check if the port is occupied and wait if necessary, then send
    while ! echo "$packet" | ncat "$TARGET_IP" "$PORT_SEND"; do
        echo "Port $PORT_SEND is occupied, retrying..." | tee -a "$LOG_FILE"
        sleep 1
    done
}

# Listen for the send packet signal and extract the packet serial
while true; do
    send_packet_signal=$(ncat -l -p $PORT_RECV)
    if [[ "$send_packet_signal" =~ SENDNOW_([0-9]+) ]]; then
        packet_serial=${BASH_REMATCH[1]}
        read time_now time_start <<< $(echo "$send_packet_signal" | awk -F'_' '{print $3, $4}')

        # Send the packet with the extracted serial
        send_packet "$packet_serial" "$time_now" "$time_start"
    elif [[ "$send_packet_signal" == "DONESEND" ]]; then
        echo "Received DONE signal, closing connection and terminating..." | tee -a "$LOG_FILE"
        break
    else
        echo "Invalid send packet signal received: $send_packet_signal" | tee -a "$LOG_FILE"
    fi
done
