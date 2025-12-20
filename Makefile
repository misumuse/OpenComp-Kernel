CC = gcc
LD = ld
CFLAGS = -O2 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -std=gnu11 -m32
LDFLAGS = -T linker.ld -nostdlib -melf_i386

OBJS = kernel.o start.o memory.o keyboard.o desktop.o

all: tinykernel.bin

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

memory.o: memory.c kernel.h
	$(CC) $(CFLAGS) -c memory.c -o memory.o

keyboard.o: keyboard.c kernel.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

desktop.o: desktop.c kernel.h
	$(CC) $(CFLAGS) -c desktop.c -o desktop.o

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

tinykernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o tinykernel.elf $(OBJS)
	@echo ""
	@echo "=== Verifying Multiboot2 Header ==="
	@if command -v grub-file > /dev/null 2>&1; then \
		if grub-file --is-x86-multiboot2 tinykernel.elf; then \
			echo "✓ Multiboot2 header found!"; \
		else \
			echo "✗ No valid multiboot2 header"; \
		fi; \
	else \
		echo "grub-file not available"; \
	fi
	@echo ""
	@echo "=== Creating Bootable ISO ==="
	mkdir -p iso/boot/grub
	cp tinykernel.elf iso/boot/kernel.elf
	echo 'set timeout=1' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "OpenComp Kernel" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.elf' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o opencomp.iso iso 2>&1 | grep -v "libgcc" || true
	@echo "✓ ISO created: opencomp.iso"

run: tinykernel.bin
	@echo "=== Starting QEMU ==="
	qemu-system-i386 -cdrom opencomp.iso -m 256M

clean:
	rm -f *.o *.elf opencomp.iso
	rm -rf iso
	@echo "✓ Cleaned build artifacts"
