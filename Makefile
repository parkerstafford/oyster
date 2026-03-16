# Oyster OS - C Kernel Makefile

CC = gcc
AS = gcc
LD = ld

CFLAGS = -Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fPIE -m64 -march=x86-64 \
         -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone

ASFLAGS = -c -m64

LDFLAGS = -nostdlib -static -pie --no-dynamic-linker -z text \
          -z max-page-size=0x1000 -T linker.ld

SRCDIR = src
OBJDIR = obj
ISODIR = iso_root

C_SOURCES = $(wildcard $(SRCDIR)/*.c)
S_SOURCES = $(wildcard $(SRCDIR)/*.S)
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
S_OBJECTS = $(patsubst $(SRCDIR)/%.S,$(OBJDIR)/%.o,$(S_SOURCES))
OBJECTS = $(C_OBJECTS) $(S_OBJECTS)

.PHONY: all clean run iso limine

all: oyster.iso

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.S | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

limine:
	@if [ ! -d "limine" ]; then \
		echo "Downloading Limine v7..."; \
		mkdir -p limine; \
		curl -L https://github.com/limine-bootloader/limine/raw/v7.x-binary/limine-bios.sys -o limine/limine-bios.sys; \
		curl -L https://github.com/limine-bootloader/limine/raw/v7.x-binary/limine-bios-cd.bin -o limine/limine-bios-cd.bin; \
		curl -L https://github.com/limine-bootloader/limine/raw/v7.x-binary/limine-uefi-cd.bin -o limine/limine-uefi-cd.bin; \
		curl -L https://github.com/limine-bootloader/limine/raw/v7.x-binary/BOOTX64.EFI -o limine/BOOTX64.EFI; \
		curl -L https://github.com/limine-bootloader/limine/raw/v7.x-binary/BOOTIA32.EFI -o limine/BOOTIA32.EFI; \
	fi

$(ISODIR): limine
	mkdir -p $(ISODIR)/boot/limine
	mkdir -p $(ISODIR)/EFI/BOOT

iso: oyster.iso

oyster.iso: kernel.elf limine limine.cfg
	mkdir -p $(ISODIR)/boot/limine
	mkdir -p $(ISODIR)/EFI/BOOT
	cp kernel.elf $(ISODIR)/boot/
	cp limine.cfg $(ISODIR)/
	cp limine.cfg $(ISODIR)/boot/limine/
	cp limine/limine-bios.sys $(ISODIR)/boot/limine/
	cp limine/limine-bios-cd.bin $(ISODIR)/boot/limine/
	cp limine/limine-uefi-cd.bin $(ISODIR)/boot/limine/
	cp limine/BOOTX64.EFI $(ISODIR)/EFI/BOOT/
	cp limine/BOOTIA32.EFI $(ISODIR)/EFI/BOOT/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISODIR) -o oyster.iso

run: oyster.iso
	qemu-system-x86_64 -cdrom oyster.iso -m 256M

run-uefi: oyster.iso
	qemu-system-x86_64 -cdrom oyster.iso -m 256M -bios /usr/share/ovmf/OVMF.fd

clean:
	rm -rf $(OBJDIR) $(ISODIR) kernel.elf oyster.iso
