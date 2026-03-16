# Oyster OS

A minimal x86_64 operating system kernel written in C.

## Prerequisites (MSYS2)

Run these commands in MSYS2 MINGW64 terminal:

```bash
# Install build tools
pacman -S git make mingw-w64-x86_64-gcc xorriso

# QEMU (if not already installed)
pacman -S mingw-w64-x86_64-qemu
```

## Building

```bash
cd /c/Users/parke/Documents/GitHub/oyster-c
make
```

This will:
1. Download the Limine bootloader (first build only)
2. Compile the kernel
3. Create a bootable ISO

## Running

```bash
make run
```

Or run QEMU directly:
```bash
qemu-system-x86_64 -cdrom oyster.iso -m 256M
```

## Project Structure

```
oyster-c/
├── Makefile          # Build system
├── limine.conf       # Bootloader configuration
├── linker.ld         # Kernel linker script
├── src/
│   ├── kernel.c      # Entry point (kmain)
│   ├── framebuffer.c # Text output to screen
│   ├── framebuffer.h
│   └── limine.h      # Limine protocol header
├── limine/           # Bootloader files (downloaded)
└── iso_root/         # ISO build directory (generated)
```

## Features

- Bare-metal x86_64 kernel
- Limine bootloader (BIOS and UEFI support)
- Framebuffer text console with bitmap font
- Higher-half kernel (loaded at 0xffffffff80000000)

## Next Steps

- Interrupt handling (IDT, exceptions)
- Memory management (physical/virtual)
- Keyboard input
- Basic shell
