ASM?=nasm
BUILD_DIR=build/

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: 
	$(ASM) kernel.asm -f bin -o $@

clean:
	rm -f $(BUILD_DIR)/kernel.bin