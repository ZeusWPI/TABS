#ifndef DRIVERS_NETWORKING_NETWORK_C
#define DRIVERS_NETWORKING_NETWORK_C

#include "e1000.c"
#include "../../memory.c"

uint8_t* create_packet(uint8_t dest[6], uint8_t src[6], uint8_t type[2], uint8_t* content, int contentlength) {
  uint8_t* returnbuffer = alloc(6 + 6 + 2 + contentlength);
  memcpy(returnbuffer, dest, 6);
  memcpy(returnbuffer + 6, src, 6);
  memcpy(returnbuffer + 6 + 6, type, 2);
  memcpy(returnbuffer + 6 + 6 + 2, content, contentlength);
  return returnbuffer;
}

void network_init() {

  uint8_t dest[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  uint8_t src[6] = {0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f};
  uint8_t type[2] = {0x69, 0x69};

  e1000_init_main();

  for (int i = 0; i < 3; i++) {
    uint8_t* packet = create_packet(dest, src, type, (uint8_t*) "Zulu Echo Uniform Sierra Whiskey Papa India", 43);

    send_packet(packet, 6 + 6 + 2 + 43);
    // TODO free(packet)
  }

  uint8_t* received_packet;
  while (1) {
    size_t s = receive_packet(&received_packet);
    if (s) {
      terminal_writeint(s, 10);
      terminal_writestring("  received packet \n");
      for (size_t i = 0; i < s; i++) {
        terminal_putchar(received_packet[i]);
      }
      // TODO free(*received_packet)
      break;
    }
  }

}

#endif // DRIVERS_NETWORKING_NETWORK_C
