CC = gcc
LD = ld
CFLAGS = -O2 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -mno-red-zone -std=gnu11 -m64
LDFLAGS = -T linker.ld -nostdlib -melf_x86_64

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
	# Verify multiboot header
	@echo "Verifying multiboot header..."
	@if command -v grub-file > /dev/null 2>&1; then \
		if grub-file --is-x86-multiboot2 tinykernel.elf; then \
			echo "✓ Multiboot2 header found!"; \
		else \
			echo "✗ Warning: No valid multiboot2 header found"; \
			echo "Checking file format..."; \
			file tinykernel.elf; \
			readelf -h tinykernel.elf 2>/dev/null || echo "readelf not available"; \
		fi \
	else \
		echo "grub-file not available, skipping verification"; \
	fi
	# create grub ISO (requires grub-mkrescue)
	mkdir -p iso/boot/grub
	cp tinykernel.elf iso/boot/kernel.elf
	printf 'set timeout=1\nset default=0\n\nmenuentry "OpenComp Kernel" {\n\tmultiboot2 /boot/kernel.elf\n\tboot\n}\n' > iso/boot/grub/grub.cfg
	grub-mkrescue -o opencomp.iso iso 2>&1 | grep -v "libgcc"

run: tinykernel.bin
	qemu-system-x86_64 -cdrom opencomp.iso -m 256M

clean:
	rm -f *.o *.elf opencomp.iso
	rm -rf iso
