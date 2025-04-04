export BASE_DIR := /opt/openwrt-sdk/openwrt-sdk-24.10.0-ramips-mt7621_gcc-13.3.0_musl.Linux-x86_64/
export STAGING_DIR := $(BASE_DIR)/staging_dir

TOOLCHAIN := $(STAGING_DIR)/toolchain-mipsel_24kc_gcc-13.3.0_musl
CROSS_COMPILE := $(TOOLCHAIN)/bin/mipsel-openwrt-linux-
ARCH := mips
CXX := $(CROSS_COMPILE)g++
CXXFLAGS := -Wall -O2 -static
LDFLAGS := -lm -lstdc++ -lgcc -lgcc_eh  # Added -lgcc_eh

SRC := statistics.cpp
OUT := statistics

all: $(OUT)

$(OUT):
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

ubuntu: clean
	$(MAKE) CXX=g++ CXXFLAGS="-Wall -O2" LDFLAGS="-lm" $(OUT)

clean:
	rm -f $(OUT)

.PHONY: all clean ubuntu