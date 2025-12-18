/* memory.c
 *
 * Memory management component for OpenComp kernel
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 *
 * Simple bitmap-based physical memory allocator
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel.h"

#define PAGE_SIZE 4096
#define TOTAL_PAGES 4096  // 16MB of manageable memory
#define BITMAP_SIZE (TOTAL_PAGES / 8)

static uint8_t page_bitmap[BITMAP_SIZE];
static uint64_t free_pages = TOTAL_PAGES;
static uint64_t base_address = 0x200000; // Start after kernel (2MB)

static void set_page_used(size_t page) {
    page_bitmap[page / 8] |= (1 << (page % 8));
}

static void set_page_free(size_t page) {
    page_bitmap[page / 8] &= ~(1 << (page % 8));
}

static int is_page_used(size_t page) {
    return (page_bitmap[page / 8] & (1 << (page % 8))) != 0;
}

void *kalloc_page(void) {
    for (size_t i = 0; i < TOTAL_PAGES; i++) {
        if (!is_page_used(i)) {
            set_page_used(i);
            free_pages--;
            void *addr = (void *)(base_address + i * PAGE_SIZE);
            // Zero the page
            uint64_t *p = (uint64_t *)addr;
            for (size_t j = 0; j < PAGE_SIZE / 8; j++) p[j] = 0;
            return addr;
        }
    }
    return NULL; // Out of memory
}

void kfree_page(void *addr) {
    uint64_t page = ((uint64_t)addr - base_address) / PAGE_SIZE;
    if (page < TOTAL_PAGES) {
        set_page_free(page);
        free_pages++;
    }
}

uint64_t get_free_pages(void) {
    return free_pages;
}

static void memory_init(void) {
    // Initialize all pages as free
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0;
    }
    puts("[memory] Physical memory manager initialized\n");
    puts("[memory] Managing ");
    char buf[32];
    itoa_u(TOTAL_PAGES * PAGE_SIZE / 1024, buf);
    puts(buf);
    puts(" KB\n");
}

static void memory_tick(void) {
    // Nothing to do each tick
}

__attribute__((section(".compobjs"))) static struct component memory_component = {
    .name = "memory",
    .init = memory_init,
    .tick = memory_tick
};

__attribute__((section(".comps"))) struct component *p_memory_component = &memory_component;
