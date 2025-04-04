export BASE_DIR := /opt/openwrt-sdk/openwrt-sdk-24.10.0-ramips-mt7621_gcc-13.3.0_musl.Linux-x86_64/
export STAGING_DIR := $(BASE_DIR)/staging_dir

TOOLCHAIN := $(BASE_DIR)/staging_dir/toolchain-mipsel_24kc_gcc-13.3.0_musl
CROSS_COMPILE := $(TOOLCHAIN)/bin/mipsel-openwrt-linux-
ARCH := mips
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -O2 -static
LDFLAGS := -lm

SRC_FIFO := fifo.c
OUT_FIFO := fifo

SRC_RR := rr.c
OUT_RR := rr

SRC_SJF := sjf.c
OUT_SJF := sjf

# Default target - cross-compile for OpenWrt
all: $(OUT_FIFO) $(OUT_RR) $(OUT_SJF)

$(OUT_FIFO): $(SRC_FIFO)
	$(CC) $(CFLAGS) -o $(OUT_FIFO) $(SRC_FIFO) $(LDFLAGS)

$(OUT_RR): $(SRC_RR)
	$(CC) $(CFLAGS) -o $(OUT_RR) $(SRC_RR) $(LDFLAGS)

$(OUT_SJF): $(SRC_SJF)
	$(CC) $(CFLAGS) -o $(OUT_SJF) $(SRC_SJF) $(LDFLAGS)

# Target for native compilation on Ubuntu
ubuntu: clean
	$(MAKE) CC=gcc CFLAGS="-Wall -O2" LDFLAGS="-lm" $(OUT_FIFO) $(OUT_RR) $(OUT_SJF)

clean:
	rm -f $(OUT_FIFO) $(OUT_RR) $(OUT_SJF)

.PHONY: all clean ubuntu