CC = gcc
LD = ld

CFLAGS32 = -O2 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -std=gnu11 -m32
LDFLAGS32 = -T linker.ld -nostdlib -melf_i386

CFLAGS64 = -O2 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -std=gnu11 -m64
LDFLAGS64 = -T linker64.ld -nostdlib -melf_x86_64

OBJS32 = kernel.o start.o memory.o keyboard.o mouse.o vga_graphics.o tarfs.o gui_desktop.o
OBJS64 = kernel64.o start64.o

all: tinykernel.bin tinykernel64.bin

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS32) -c kernel.c -o kernel.o

memory.o: memory.c kernel.h
	$(CC) $(CFLAGS32) -c memory.c -o memory.o

keyboard.o: keyboard.c kernel.h
	$(CC) $(CFLAGS32) -c keyboard.c -o keyboard.o

desktop.o: desktop.c kernel.h
	$(CC) $(CFLAGS32) -c desktop.c -o desktop.o

mouse.o: mouse.c kernel.h
	$(CC) $(CFLAGS32) -c mouse.c -o mouse.o

vga_graphics.o: vga_graphics.c kernel.h
	$(CC) $(CFLAGS32) -c vga_graphics.c -o vga_graphics.o

tarfs.o: tarfs.c kernel.h
	$(CC) $(CFLAGS32) -c tarfs.c -o tarfs.o

gui_desktop.o: gui_desktop.c kernel.h
	$(CC) $(CFLAGS32) -c gui_desktop.c -o gui_desktop.o

start.o: start.S
	$(CC) $(CFLAGS32) -c start.S -o start.o

kernel64.o: kernel64.c
	$(CC) $(CFLAGS64) -c kernel64.c -o kernel64.o

start64.o: start64.S
	$(CC) $(CFLAGS64) -c start64.S -o start64.o

tinykernel.bin: $(OBJS32) linker.ld
	$(LD) $(LDFLAGS32) -o tinykernel.elf $(OBJS32)
	@echo ""
	@echo "=== Verifying Multiboot2 Header (32-bit) ==="
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
	@echo "=== Creating 32-bit Bootable ISO ==="
	mkdir -p iso/boot/grub
	cp tinykernel.elf iso/boot/kernel.elf
	echo 'set timeout=1' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "OpenComp Kernel (32-bit)" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.elf' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o opencomp.iso iso 2>&1 | grep -v "libgcc" || true
	@echo "✓ ISO created: opencomp.iso"

tinykernel64.bin: $(OBJS64) linker64.ld
	$(LD) $(LDFLAGS64) -o tinykernel64.elf $(OBJS64)
	@echo ""
	@echo "=== Creating x86_64 ISO ==="
	mkdir -p iso64/boot/grub
	cp tinykernel64.elf iso64/boot/kernel64.elf
	echo 'set timeout=1' > iso64/boot/grub/grub.cfg
	echo 'set default=0' >> iso64/boot/grub/grub.cfg
	echo '' >> iso64/boot/grub/grub.cfg
	echo 'menuentry "OpenComp Kernel (x86_64 experimental)" {' >> iso64/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel64.elf' >> iso64/boot/grub/grub.cfg
	echo '    boot' >> iso64/boot/grub/grub.cfg
	echo '}' >> iso64/boot/grub/grub.cfg
	grub-mkrescue -o opencomp-x86_64.iso iso64 2>&1 | grep -v "libgcc" || true
	@echo "✓ ISO created: opencomp-x86_64.iso"

run: tinykernel.bin
	@echo "=== Starting QEMU (i386) ==="
	@echo "Click in window to grab mouse, Ctrl+Alt+G to release"
	qemu-system-i386 -cdrom opencomp.iso -m 256M

run64: tinykernel64.bin
	@echo "=== Starting QEMU (x86_64) ==="
	qemu-system-x86_64 -cdrom opencomp-x86_64.iso -m 256M

clean:
	rm -f *.o *.elf opencomp.iso opencomp-x86_64.iso
	rm -rf iso iso64
	@echo "✓ Cleaned build artifacts"
