CC = x86_64-elf-gcc
LD = x86_64-elf-ld
CFLAGS = -O2 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -mno-red-zone -std=gnu11
LDFLAGS = -T linker.ld

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
	# create grub ISO (requires grub-mkrescue)
	mkdir -p iso/boot/grub
	cp tinykernel.elf iso/boot/kernel.elf
	printf 'set timeout=1\nmenuentry "OpenComp" { multiboot /boot/kernel.elf }\n' > iso/boot/grub/grub.cfg
	grub-mkrescue -o opencomp.iso iso

run: tinykernel.bin
	qemu-system-x86_64 -cdrom opencomp.iso -m 256M

clean:
	rm -f *.o *.elf opencomp.iso
	rm -rf iso
