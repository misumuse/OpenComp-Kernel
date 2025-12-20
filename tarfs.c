/* tarfs.c
 *
 * Simple TAR filesystem for initrd
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 *
 * TAR format: 512-byte headers followed by file data
 */

#include <stdint.h>
#include "kernel.h"

#define TAR_BLOCK_SIZE 512
#define MAX_FILES 32

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
} __attribute__((packed)) tar_header_t;

typedef struct {
    char name[128];
    uint32_t size;
    uint8_t *data;
    int is_dir;
} file_entry_t;

static file_entry_t files[MAX_FILES];
static int file_count = 0;
static uint8_t *initrd_start = NULL;
static uint32_t initrd_size = 0;

// Convert octal string to integer
static uint32_t parse_octal(const char *str, int len) {
    uint32_t result = 0;
    for (int i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++) {
        result = result * 8 + (str[i] - '0');
    }
    return result;
}

// Copy string safely
static void safe_strcpy(char *dest, const char *src, int max_len) {
    int i;
    for (i = 0; i < max_len - 1 && src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = 0;
}

// Parse TAR archive
static void parse_tar(uint8_t *tar_data, uint32_t size) {
    uint8_t *ptr = tar_data;
    file_count = 0;
    
    while (ptr < tar_data + size && file_count < MAX_FILES) {
        tar_header_t *header = (tar_header_t *)ptr;
        
        // Check for end of archive (null header)
        if (header->name[0] == 0) break;
        
        // Check magic number
        if (header->magic[0] != 'u' || header->magic[1] != 's' ||
            header->magic[2] != 't' || header->magic[3] != 'a' ||
            header->magic[4] != 'r') {
            // Try old tar format (no magic)
            if (header->name[0] == 0) break;
        }
        
        uint32_t file_size = parse_octal(header->size, 12);
        
        // Store file info
        safe_strcpy(files[file_count].name, header->name, 128);
        files[file_count].size = file_size;
        files[file_count].data = ptr + TAR_BLOCK_SIZE;
        files[file_count].is_dir = (header->typeflag == '5');
        
        file_count++;
        
        // Move to next entry (header + data, rounded up to 512 bytes)
        uint32_t blocks = (file_size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        ptr += TAR_BLOCK_SIZE + (blocks * TAR_BLOCK_SIZE);
    }
}

// Get file count
int fs_get_file_count(void) {
    return file_count;
}

// Get file info by index
int fs_get_file_info(int index, char *name, uint32_t *size, int *is_dir) {
    if (index < 0 || index >= file_count) return 0;
    
    safe_strcpy(name, files[index].name, 128);
    *size = files[index].size;
    *is_dir = files[index].is_dir;
    return 1;
}

// Read file by name
int fs_read_file(const char *filename, uint8_t **data, uint32_t *size) {
    for (int i = 0; i < file_count; i++) {
        // Simple name comparison
        int match = 1;
        for (int j = 0; j < 128; j++) {
            if (files[i].name[j] != filename[j]) {
                match = 0;
                break;
            }
            if (files[i].name[j] == 0 && filename[j] == 0) break;
        }
        
        if (match) {
            *data = files[i].data;
            *size = files[i].size;
            return 1;
        }
    }
    return 0;
}

// Read file by index
int fs_read_file_by_index(int index, uint8_t **data, uint32_t *size) {
    if (index < 0 || index >= file_count) return 0;
    
    *data = files[index].data;
    *size = files[index].size;
    return 1;
}

// Set initrd location (called from kernel with multiboot info)
void fs_set_initrd(uint8_t *addr, uint32_t size) {
    initrd_start = addr;
    initrd_size = size;
    
    if (initrd_start && initrd_size > 0) {
        parse_tar(initrd_start, initrd_size);
        
        puts("[tarfs] Initrd loaded at ");
        char buf[32];
        itoa_u((uint64_t)initrd_start, buf);
        puts(buf);
        puts(", size: ");
        itoa_u(initrd_size, buf);
        puts(buf);
        puts(" bytes\n");
    }
}

static void tarfs_init(void) {
    puts("[tarfs] TAR filesystem driver initialized\n");
    
    // For now, create a fake initrd with some test files
    // In a real implementation, this would come from multiboot
    
    // Create a simple test "filesystem" in memory
    puts("[tarfs] Creating test filesystem...\n");
    
    // We'll add real initrd loading later
    // For now, just create some fake file entries
    
    safe_strcpy(files[0].name, "readme.txt", 128);
    files[0].size = 50;
    files[0].data = (uint8_t*)"Welcome to OpenComp!\nThis is a test file.\n";
    files[0].is_dir = 0;
    
    safe_strcpy(files[1].name, "hello.txt", 128);
    files[1].size = 30;
    files[1].data = (uint8_t*)"Hello from the filesystem!";
    files[1].is_dir = 0;
    
    safe_strcpy(files[2].name, "docs/", 128);
    files[2].size = 0;
    files[2].data = NULL;
    files[2].is_dir = 1;
    
    safe_strcpy(files[3].name, "docs/info.txt", 128);
    files[3].size = 35;
    files[3].data = (uint8_t*)"Documentation goes here.\nMore info!";
    files[3].is_dir = 0;
    
    file_count = 4;
    
    puts("[tarfs] Test filesystem created with ");
    char buf[32];
    itoa_u(file_count, buf);
    puts(buf);
    puts(" files\n");
}

static void tarfs_tick(void) {
    // Nothing to do
}

__attribute__((section(".compobjs"))) static struct component tarfs_component = {
    .name = "tarfs",
    .init = tarfs_init,
    .tick = tarfs_tick
};

__attribute__((section(".comps"))) struct component *p_tarfs_component = &tarfs_component;
