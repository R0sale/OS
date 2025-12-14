BUILD_DIR=build

ASM=nasm

.PHONY: all main.img clean bootloader kernel

all: main.img

main.img: $(BUILD_DIR)/main.img

$(BUILD_DIR)/main.img: bootloader kernel
  dd if=/dev/zero of=$(BUILD_DIR)/main.img bs=512 count=65536
  mkfs.fat -F 16 -n "NBOS" $(BUILD_DIR)/main.img
  dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main.img conv=notrunc bs=1 count=3
  dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main.img conv=notrunc seek=62 skip=62
  mcopy -i $(BUILD_DIR)/main.img $(BUILD_DIR)/kernel.bin "::kernel.bin" 

bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: always
  $(ASM) bootloader/boot.asm -f bin -o $(BUILD_DIR)/bootloader.bin

kernel:  $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
  $(ASM) kernel/kernel.asm -f bin -o $(BUILD_DIR)/kernel.bin

always:
  mkdir -p $(BUILD_DIR)

clean:
  rm -f $(BUILD_DIR)/*