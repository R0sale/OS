BUILD_DIR=build

ASM=nasm

.PHONY: all floppy_img clean bootloader kernel

all: floppy_img

floppy_img: $(BUILD_DIR)/floppy.img

$(BUILD_DIR)/floppy.img: bootloader kernel
  dd if=/dev/zero of=$(BUILD_DIR)/floppy.img bs=512 count=2880
  dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/floppy.img conv=notrunc
  dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/floppy.img seek=1 conv=notrunc

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