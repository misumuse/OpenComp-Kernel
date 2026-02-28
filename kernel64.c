/* kernel64.c
 * Minimal x86_64 kernel payload for experimental ISO builds.
 */

#include <stdint.h>

static volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;

void kernel64_main(void) {
    const char *msg = "OpenComp x86_64 experimental build";
    for (int i = 0; msg[i]; i++) {
        vga[i] = (uint16_t)msg[i] | (0x0F << 8);
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
