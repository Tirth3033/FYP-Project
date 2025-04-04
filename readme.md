## How to Use/Run It

---

## Directory Structure
```
project_root/
│-- router/               # Router-side scripts
│-- rpi/                  # Raspberry Pi scripts
│-- docker/               # Docker setup for OpenWRT SDK
│-- desktop/              # C source code for executables
```

---

To properly execute the packet scheduler system, follow these steps:

### 1. Setup Network and Devices
- Ensure all **4 Raspberry Pis (RPis) and the router** are powered on and connected to the **same LAN**.
- The router must have **OpenWRT of a specific version** installed and be **SSH-accessible** from all 5 machines (RPis + Router).

### 2. Deploy Required Files
- Copy the `rpi/` folder to **each Raspberry Pi**.
- Copy the `router/` folder to the **router**.

### 3. Run Commands with Root Privileges
- All execution commands must be run with **sudo privileges** to ensure proper functionality.

### 4. Scheduler Executables on Router
- The router expects a **subdirectory named `schedulers/`** in the same parent directory where it is executed.
- This folder should contain **all the scheduler executables**, which must be cross-compiled for the **specific OpenWRT version** installed on the router.
- The **name of each scheduler executable must match** the algorithm name that will be used in the commands.
- Any **custom scheduler must also be placed in the same `schedulers/` directory** following this naming rule.
- **RPis do not require any scheduler executables**.

### 5. Start the RPis
Run the following command on **each RPi sequentially**:
```sh
sudo ./start.sh -m [min_pkt_sz] -M [max_pkt_sz] -n [packet_cnt]
```
**Parameters:**
- `-m [min_pkt_sz]` - Minimum packet size (optional, default set)
- `-M [max_pkt_sz]` - Maximum packet size (optional, default set)
- `-n [packet_cnt]` - Number of packets (optional, default set)

**Note:** If parameters are omitted, the default values will be used.

### 6. RPis Wait for Router Initialization
After running the command, the **RPis will start and wait** for an initialization signal from the router.

### 7. Start the Experiment on the Router
Run the following command on the **router**:
```sh
sudo ./start.sh -a [algo_name]
```
- `algo_name` must **exactly match** the name of the **scheduler executable** present in the `schedulers/` directory.

### 8. Execution and Log Generation
- The **experiment will start** and automatically coordinate all necessary actions across the RPis and the router.
- The following **log files will be generated**:
  - **Router:** `receiver_log.txt`
  - **Each RPi:** `sender_log.txt`
  - **Router:** `ip_1_stats.log`, `ip_2_stats.log`, `ip_3_stats.log`, `ip_4_stats.log`
  - **Router:** `combined_stats.log` (contains consolidated statistics analysis)

# Packet Scheduler System - README

## Overview
This project implements a packet scheduling system involving a router and multiple Raspberry Pis (RPis). The system consists of a scheduler that reorders packets based on a specified algorithm before forwarding them. The project is structured across multiple directories for different components.


## How to Set Up Various Docker Images for Testing

The `desktop/` and `docker/` directories should be used on any laptop or desktop with an operating system such as Linux, Windows, or macOS, provided that Docker Desktop is installed and enabled in the system PATH.

### Steps:
1. Navigate to the `docker/` directory.
2. Place the `sdk.tar.zst` file inside this directory.
3. Open a terminal or PowerShell in this directory and run:
   ```sh
   docker-compose up
   ```
   This will automatically build the required Docker image and create a container named `openwrt_sdk`, based on Ubuntu, with all paths and variables preconfigured for cross-compiling the executables.
4. If needed, navigate to the `desktop/` directory and run:
   ```sh
   docker-compose up
   ```
   This will create a virtual LAN containing a router and four RPi containers, which can be used to edit or test code in an Ubuntu environment.
5. The container names can be found by running:
   ```sh
   docker ps -a
   ```
   Any container that needs to be started should be started using:
   ```sh
   docker start -i [container_name]
   ```


## How to Cross-Compile Executables

The `desktop/` folder contains two subdirectories: `schedulers/` and `statistics/`. These directories contain the C source files for the scheduling algorithms and statistical analysis tools, respectively. Each directory also contains a `Makefile` that defines how the executables should be compiled.

### Steps:
1. If any executables are already present, open a terminal in the respective directory and run:
   ```sh
   make clean
   ```
   This ensures a fresh build by removing any previous executables and object files.
2. To add a new scheduler algorithm, place its `.c` file inside the `schedulers/` directory.
3. If required, modify the `Makefile` to include the new source file. If unsure, you can use ChatGPT to help rewrite the `Makefile` accordingly.
4. Once the `Makefile` is updated, compile all executables by running:
   ```sh
   make
   ```
   This will generate executables for all C source files in the directory.
5. If you need to modify `statistics.c`, make the necessary changes, and update the `Makefile` if needed.
6. Run `make` again in the `statistics/` directory to regenerate the statistics executable.
7. After compilation:
   - Move all scheduler executables to the `schedulers/` directory inside the `router/` folder.
   - Move the statistics executable to the `router/` folder.

These compiled executables will now work with the `start.sh` script on the router.

### Shortcut for Cross-Compiling Schedulers

In case of schedulers, you can follow a shortcut method to cross-compile and set up the executables:

1. Place the `.c` file inside the `schedulers/` directory.
2. Edit the `Makefile` to include the new source file if required.
3. Navigate back to the `desktop/` directory.
4. Run the following command for cross-compiling and setup:
   ```sh
   ./compile.sh c
   ```
5. Alternatively, if you want to compile for Ubuntu and set up, run:
   ```sh
   ./compile.sh u
   ```

This will handle the compilation and setup process for you efficiently.


---

## Setting Up Containers Inside Docker

1. Once inside any of the containers, it will contain all four folders in this project.
2. Navigate to the root directory of the project where these four folders are present and run:
   ```sh
   find . -type f -exec dos2unix {} +
   ```
   This ensures all files are in the correct format for execution.
3. Change the permissions of all folders and files recursively by running:
   ```sh
   chmod -R 777 .
   ```


