#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define KERNEL_SECTORS 64

int main(int argc, char** argv) {
    FILE* result_file = fopen("./target/boot.bin", "w+b");

    FILE* bootloader_file = fopen("./target/bootloader/bootloader.bin", "rb");

    // ------- BOOTLOADER + SFS HEADER -------
    char* buffer = calloc(512, sizeof(char));
    int i = 0;
    int curr_char;

    while ((curr_char = fgetc(bootloader_file)) != EOF) {
        buffer[i] = (char) curr_char;
        i++;
    }

    // TODO edit SFS header

    buffer[0x194] = 0xff;


    for (int i = 0; i < 512; i++) {
        fputc(buffer[i], result_file);
    }

    fclose(bootloader_file);


    // ------- KERNEL -------
    
    FILE* kernel_file = fopen("./target/kernel/kernel.bin", "rb");

    int bytes_written = 0;

    while ((curr_char = fgetc(kernel_file)) != EOF) {
        fputc((char) curr_char, result_file);
        bytes_written++;
    }

    for (; bytes_written < KERNEL_SECTORS * 512; bytes_written++) {
        fputc(0, result_file);
    }


    fclose(result_file);

    return EXIT_SUCCESS;
}