ASM=nasm
CC=gcc
BUILD_DIR=build
TOOLS_DIR=tools

.PHONY: all main.img bootloader stage1 stage2 kernel tools_fat always clean

all: main.img

main.img: $(BUILD_DIR)/main.img

$(BUILD_DIR)/main.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=65536
	mkfs.fat -F 16 -n "NBOS" $(BUILD_DIR)/main.img
	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main.img conv=notrunc bs=1 count=3
	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main.img conv=notrunc bs=1 seek=62 skip=62
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD_DIR)/main.img test.txt "::test.txt"
	mmd -i $(BUILD_DIR)/main.img "::mydir"
	mcopy -i $(BUILD_DIR)/main.img test2.txt "::mydir/test2.txt"

bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	$(MAKE) -C bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	$(MAKE) -C bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin:
	$(MAKE) -C kernel BUILD_DIR=$(abspath $(BUILD_DIR))

tools_fat: $(BUILD_DIR)/tools/fat

$(BUILD_DIR)/tools/fat: always $(TOOLS_DIR)/fat/fat.c
	mkdir -p $(BUILD_DIR)/tools
	$(CC) -g -o $(BUILD_DIR)/tools/fat $(TOOLS_DIR)/fat/fat.c

always:
	mkdir -p $(BUILD_DIR)

clean:
	$(MAKE) -C bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	rm -f $(BUILD_DIR)/*
