#include "EdgeVision.h"

// Global variables for memory-mapped addresses
volatile uint32_t *pixel_in_pio = NULL;
volatile uint8_t *pixel_out_pio = NULL;
void *lw_bridge_base = NULL;
int fd = -1;

/**
 * Configure the Lightweight HPS-to-FPGA bridge and map PIOs.
 * Returns 0 on success, -1 on failure.
 */
int configure_fpga() {
    // Open memory-mapped device
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // Map the Lightweight HPS-to-FPGA bridge into user space
    lw_bridge_base = mmap(NULL, LW_BRIDGE_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (lw_bridge_base == MAP_FAILED) {
        perror("Error mapping LW bridge");
        close(fd);
        return -1;
    }

    // Calculate PIO addresses
    pixel_in_pio = (uint32_t *)((uintptr_t)lw_bridge_base + PIXEL_IN_PIO_BASE);
    pixel_out_pio = (uint8_t *)((uintptr_t)lw_bridge_base + PIXEL_OUT_PIO_BASE);

    return 0;
}

/**
 * Write data to the FPGA via the pixel_in_pio.
 */
void write_to_fpga(uint32_t data) {
    if (pixel_in_pio != NULL) {
        *pixel_in_pio = data;
        //printf("HPS -> FPGA: Sent 0x%X\n", data);
    } else {
        fprintf(stderr, "Error: PIO not configured. Call configure_fpga() first.\n");
    }
}

/**
 * Read data from the FPGA via the pixel_out_pio.
 */
uint8_t read_from_fpga() {
    if (pixel_out_pio != NULL) {
        uint8_t data = *pixel_out_pio;
        //printf("FPGA -> HPS: Received 0x%X\n", data);
        return data;
    } else {
        fprintf(stderr, "Error: PIO not configured. Call configure_fpga() first.\n");
        return 0;
    }
}

/**
 * Cleanup function to unmap memory and close the file descriptor.
 */
void cleanup_fpga() {
    if (lw_bridge_base != NULL) {
        munmap(lw_bridge_base, LW_BRIDGE_SPAN);
        lw_bridge_base = NULL;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}
