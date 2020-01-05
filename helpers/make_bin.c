#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define KERNEL_SECTORS 64

#define KiB 1024
#define MiB (1024 * KiB)
#define GiB (1024 * MiB)

#define DISK_SIZE (16 * MiB)

#define BLOCK_SIZE_ARG 1
#define BLOCK_SIZE (1 << (BLOCK_SIZE_ARG + 7)) // 256 bytes

#define RESERVED_SIZE (32 * KiB)
#define NUM_RESERVED_BLOCKS (RESERVED_SIZE / BLOCK_SIZE)

#define DISK_NAME "RobUSB"

uint8_t* buffer;
long buffer_index = 0;

uint8_t* index_start;
int index_entries = 0;

int block_entries = 0;

void add_directory(const char* directory_name) {
    index_start[0x00] = 0x11; // It's a folder

    uint8_t num_continuations;

    if (strlen(directory_name) < 30) {
        num_continuations = 0; // No continuation needed
    } else {
        num_continuations = ((strlen(directory_name) - 30) / 64) + 1;
    }

    index_start[0x01] = num_continuations;

    time_t current_time = time(NULL);
    uint64_t sfs_timestamp = (uint64_t) current_time * 65536;

    *(uint64_t*) (index_start + 0x02) = sfs_timestamp; // Add time

    int pos = 0x0A;

    for (int i = 0; i < strlen(directory_name); i++) {
        index_start[pos] = directory_name[i];
        pos++;
        if (pos >= 64) {
            pos = 0;
            index_start -= 64;
            index_entries++;
        }
    }

    index_start -= 64;
    index_entries++;

}

void add_file(const char* filename) {
    FILE* source = fopen(filename, "rb");

    // Add to Index area
    index_start[0x00] = 0x12; // It's a file

    uint8_t num_continuations;

    if (strlen(filename) < 30) {
        num_continuations = 0; // No continuation needed
    } else {
        num_continuations = ((strlen(filename) - 30) / 64) + 1;
    }

    index_start[0x01] = num_continuations;

    time_t current_time = time(NULL);
    uint64_t sfs_timestamp = (uint64_t) current_time * 65536;

    *(uint64_t*) (index_start + 0x02) = sfs_timestamp; // Add time
    *(uint64_t*) (index_start + 0x0a) = *(uint64_t*) (buffer + 0x019C); // Data block start


    // Add to Data area
    char curr_char;

    int i = 0;
    long size = 0;

    while((curr_char = fgetc(source)) != EOF) {
        buffer[buffer_index] = (uint8_t) curr_char;
        buffer_index++;
        size++;
        if (i >= 255) {
            i = 0;
            (*(uint64_t*) (buffer + 0x019C))++;
        } else {
            i++;
        }
    }

    buffer[0x019C]++;
    buffer_index += (BLOCK_SIZE - (buffer_index % BLOCK_SIZE));

    // Rest of Index area
    *(uint64_t*) (index_start + 0x12) = *(uint64_t*) (buffer + 0x019C); // Data block end
    *(uint64_t*) (index_start + 0x1A) = (uint64_t) size;

    int pos = 0x22;

    for (int i = 0; i < strlen(filename); i++) {
        index_start[pos] = filename[i];
        pos++;
        if (pos >= 64) {
            pos = 0;
            index_start -= 64;
            index_entries++;
        }
    }

    index_start -= 64;
    index_entries++;

    fclose(source);
}

void add_files() {
    add_directory("filesystem");
    add_file("filesystem/test.txt");
    add_directory("filesystem/test_folder_with_long_name_that_is_actually_quite_unpractical_to_type");
    add_file("filesystem/test_folder_with_long_name_that_is_actually_quite_unpractical_to_type/test_file_with_very_VERY_long_name");
}

int main(int argc, char** argv) {
    buffer = calloc(DISK_SIZE, sizeof(uint8_t));

    FILE* bootloader_file = fopen("./target/bootloader/bootloader.bin", "rb");

    // ------- BOOTLOADER + SFS HEADER -------
    int curr_byte;

    while ((curr_byte = fgetc(bootloader_file)) != EOF) {
        buffer[buffer_index] = (uint8_t) curr_byte;
        buffer_index++;
    }

    // SFS Superblock
    time_t current_time = time(NULL);
    uint64_t sfs_timestamp = (uint64_t) current_time * 65536;

    (*(uint64_t*) (buffer + 0x194)) = sfs_timestamp;
    (*(uint64_t*) (buffer + 0x19C)) = 0; // Start off with empty Data-area
    (*(uint64_t*) (buffer + 0x1A4)) = 0; // Start off with basic Index-area
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

    fclose(bootloader_file);


    // ------- KERNEL -------
    
    FILE* kernel_file = fopen("./target/kernel/kernel.bin", "rb");

    int bytes_written = 0;

    while ((curr_byte = fgetc(kernel_file)) != EOF) {
        buffer[buffer_index] = (char) curr_byte;
        buffer_index++;
    }

    buffer_index = ((KERNEL_SECTORS + 1) * 512);

    fclose(kernel_file);


    // ------- FILESYSTEM -------

    // Volume identifier
    index_start = buffer + DISK_SIZE - 64;

    index_start[0x00] = 0x01; // It's a volume identifier
    (*(uint64_t*) (index_start + 0x04)) = sfs_timestamp;
    
    for (int i = 0; i < strlen(DISK_NAME); i++)
    {
        index_start[0x0c + i] = DISK_NAME[i];
    }

    index_start -= 64; // next entry
    index_entries++;

    // Files
    add_files();

    // Starting marker
    index_start[0x00] = 0x02; // It's a starting marker

    index_start -= 64; // next entry
    index_entries++;

    // ------- WRITE BIN -------

    FILE* result_file = fopen("./target/boot.bin", "w+b");

    fwrite(buffer, sizeof(uint8_t), DISK_SIZE, result_file);

    fclose(result_file);

    free(buffer);

    return EXIT_SUCCESS;
}