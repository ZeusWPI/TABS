#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define KERNEL_SECTORS 64

#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define DISK_SIZE (1 * GiB)

#define BLOCK_SIZE_ARG 1
#define BLOCK_SIZE (1 << (BLOCK_SIZE_ARG + 7))

#define RESERVED_SIZE (32 * KiB)
#define NUM_RESERVED_BLOCKS (RESERVED_SIZE / BLOCK_SIZE)

#define DATA_SIZE (1 * GiB)
#define NUM_DATA_BLOCKS (DATA_SIZE / BLOCK_SIZE)

#define INDEX_SIZE (128 * MiB)

int main(int argc, char** argv) {
    FILE* result_file = fopen("./target/boot.bin", "w+b");

    FILE* bootloader_file = fopen("./target/bootloader/bootloader.bin", "rb");

    // ------- BOOTLOADER + SFS HEADER -------
    uint8_t* buffer = calloc(512, sizeof(uint8_t));
    int i = 0;
    int curr_byte;

    while ((curr_byte = fgetc(bootloader_file)) != EOF) {
        buffer[i] = (uint8_t) curr_byte;
        i++;
    }

    // SFS Superblock
    time_t current_time = time(NULL);
    uint64_t sfs_timestamp = (uint64_t) current_time * 65536;

    (*(uint64_t*) (buffer + 0x194)) = sfs_timestamp;
    (*(uint64_t*) (buffer + 0x19C)) = NUM_DATA_BLOCKS;
    (*(uint64_t*) (buffer + 0x1A4)) = INDEX_SIZE;
    (*(uint32_t*) (buffer + 0x1AC)) = 0x10534653; // SFS magic + version
    (*(uint64_t*) (buffer + 0x1B0)) = ((uint64_t) DISK_SIZE) / BLOCK_SIZE;

    // The superblock may be bigger than one block
    if (BLOCK_SIZE < 512) {
        (*(uint32_t*) (buffer + 0x1B8)) = NUM_RESERVED_BLOCKS + 2;
    } else {
        (*(uint32_t*) (buffer + 0x1B8)) = NUM_RESERVED_BLOCKS + 1;
    }
    buffer[0x1BC] = BLOCK_SIZE_ARG;

    uint8_t sum = 0;
    for (int i = 0x1AC; i < 0x1BD; i++) {
        sum += buffer[i];
    }
    buffer[0x1BD] = -sum;


    for (int i = 0; i < 512; i++) {
        fputc(buffer[i], result_file);
    }

    fclose(bootloader_file);


    // ------- KERNEL -------
    
    FILE* kernel_file = fopen("./target/kernel/kernel.bin", "rb");

    int bytes_written = 0;

    while ((curr_byte = fgetc(kernel_file)) != EOF) {
        fputc((char) curr_byte, result_file);
        bytes_written++;
    }

    for (; bytes_written < KERNEL_SECTORS * 512; bytes_written++) {
        fputc(0, result_file);
    }


    fclose(result_file);

    return EXIT_SUCCESS;
}