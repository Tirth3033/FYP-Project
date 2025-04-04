# 🛰️ Raspberry Pi – OpenWRT Router Scheduling Testbed

This repository contains a complete implementation of a physical testbed designed to evaluate network packet scheduling algorithms on a real embedded system. The testbed uses a Xiaomi Mi Router 4A Gigabit Edition (running OpenWRT) as the central coordinator and multiple Raspberry Pi 5 units as clients. The project implements and analyzes FIFO, SJF, and Round Robin (RR) scheduling policies under varying network loads.

---

## 📁 Project Structure

```
FYP-Project/
│
├── rpi/                      # Scripts for Raspberry Pi clients
├── router/                   # Router-side scripts and log handlers
├── docker/                   # Dockerfile and SDK for cross-compilation
├── report_experiment_results_and_images/
├── combined_stats.xlsx       # Final statistics from experiments
├── README.md
```

---

## 📦 Prerequisites

### Hardware
- Xiaomi Mi Router 4A Gigabit Edition
- 3–4 Raspberry Pi 5 units
- SD cards with Raspberry Pi OS
- USB card reader
- PC with macOS or Linux (for flashing, scripting, compilation)

---

## 🧩 1. Installing OpenWRT on Xiaomi Router

> Followed [OpenWRTInvasion GitHub](https://github.com/acecilia/OpenWRTInvasion) and [YouTube guide](https://www.youtube.com/watch?v=54ehu3u8vXc)

### 🔧 Steps:
1. Connect router to PC via LAN.
2. Use a clean Ubuntu/macOS system and install Docker:
   ```bash
   brew install docker
   ```
3. Clone the OpenWRTInvasion repo:
   ```bash
   git clone https://github.com/acecilia/OpenWRTInvasion
   cd OpenWRTInvasion
   sudo docker-compose up -d
   ```
4. Gain root access to router using the invasion script.
5. Transfer OpenWRT firmware:
   ```bash
   curl http://<your-ip>:8000/kernel1.bin --output kernel1.bin
   curl http://<your-ip>:8000/rootfs0.bin --output rootfs0.bin
   mtd write kernel1.bin kernel1
   mtd write rootfs0.bin rootfs0
   reboot
   ```
6. Flash the OpenWRT sysupgrade `.bin` via the LuCI GUI.

---

## 🔧 2. Raspberry Pi Setup (Headless)

### 🛠️ Steps:
1. Flash Raspberry Pi OS using Raspberry Pi Imager:
   - Enable SSH and Wi-Fi from Advanced settings
   - Use SD card reader
2. Boot Pi and connect to router via Wi-Fi.
3. Open GUI to set static IP:
   - Examples: `192.168.1.230` to `192.168.1.234`
4. Set hostname and create a new user (`tirthoza`)
5. Install dependencies:
   ```bash
   sudo apt update
   sudo apt install ncat
   ```

6. Copy over the `start.sh` script from the `rpi/` folder and make executable:
   ```bash
   scp start.sh pi@192.168.1.230:/home/pi/scripts/
   chmod +x start.sh
   ```

---

## 🧰 3. Cross-Compiling C Code for Router

To build scheduling binaries (e.g., `fifo`, `sjf`, `rr`) compatible with the router's MIPS architecture, use the OpenWRT SDK.

### 🔩 SDK Setup (Git LFS required)

#### Step 1: Install Git LFS

```bash
brew install git-lfs
git lfs install
```

#### Step 2: Clone this repo and pull LFS files

```bash
git clone https://github.com/Tirth3033/FYP-Project.git
cd FYP-Project
git lfs pull
```

You’ll now see:

```
docker/sdk.tar.zst
```

#### Step 3: Extract and build

```bash
cd docker
tar -xf sdk.tar.zst
# Follow Dockerfile instructions or enter SDK shell
```

Use the provided `Makefile` and source files to compile:
```bash
make fifo
make sjf
make rr
```

Place the resulting binaries in the `router/schedulers/` folder.

---

## 🖧 4. System Communication & Execution

- The router acts as the **central controller**.
- RPis wait for control messages:
  - `STARTNOW` → Begin metadata sharing
  - `SENDNOW` → Begin data transfer
  - `DONESEND` → Completion notice

### Running a Test
On the router:

```bash
sh start.sh -a <fifo|sjf|rr>
```

> Ensure static IPs of RPis are hardcoded correctly in `start.sh`.

Logs:
- Router: `receiver_log.txt`
- RPis: `sender_log.txt`

---

## 📊 Experiment Results

Located in:
- `combined_stats.xlsx`
- `/report_experiment_results_and_images/`

Metrics:
- Throughput
- Latency
- Jitter
- Fairness
- Packet Loss

---

## 📚 References

- GitHub: [OpenWRTInvasion](https://github.com/acecilia/OpenWRTInvasion)
- YouTube: [Van Tech Corner - OpenWRT Install](https://www.youtube.com/watch?v=54ehu3u8vXc)
- Raspberry Pi: [Official Imaging Tool](https://www.raspberrypi.com/software/)
- Git LFS: [https://git-lfs.github.com](https://git-lfs.github.com)

---

## 📜 License

This project is developed for academic purposes and is shared under an open-source MIT License unless otherwise specified.
