#ifndef DRIVERS_NETWORKING_E1000_C
#define DRIVERS_NETWORKING_E1000_C

#include "../../inline_asm.c"
#include "../pci/pci.c"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

static uint32_t e1000_device_pci = 0x00000000;
static uintptr_t mem_base = 0;
static int has_eeprom = 0;
static uint8_t e1000_mac[6];

// Aligned alloc
void* valloc(unsigned int size, int i) {
  uint32_t addr = (uint32_t) alloc(size + (1 << i));
  addr = (((addr - 1) >> i) + 1 ) << i;
  return (void*) addr;
}

struct rx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint16_t checksum;
	volatile uint8_t  status;
	volatile uint8_t  errors;
	volatile uint16_t special;
} __attribute__((packed));

struct tx_desc {
	volatile uint64_t addr;
	volatile uint16_t length;
	volatile uint8_t  cso;
	volatile uint8_t  cmd;
	volatile uint8_t  status;
	volatile uint8_t  css;
	volatile uint16_t special;
} __attribute__((packed));

static uint8_t * rx_virt[E1000_NUM_RX_DESC];
static uint8_t * tx_virt[E1000_NUM_TX_DESC];
static struct rx_desc * rx;
static struct tx_desc * tx;
static uintptr_t rx_phys;
static uintptr_t tx_phys;

#define E1000_REG_CTRL       0x0000
#define E1000_REG_STATUS     0x0008
#define E1000_REG_EEPROM     0x0014
#define E1000_REG_CTRL_EXT   0x0018

#define E1000_REG_RCTRL      0x0100
#define E1000_REG_RXDESCLO   0x2800
#define E1000_REG_RXDESCHI   0x2804
#define E1000_REG_RXDESCLEN  0x2808
#define E1000_REG_RXDESCHEAD 0x2810
#define E1000_REG_RXDESCTAIL 0x2818

#define E1000_REG_TCTRL      0x0400
#define E1000_REG_TXDESCLO   0x3800
#define E1000_REG_TXDESCHI   0x3804
#define E1000_REG_TXDESCLEN  0x3808
#define E1000_REG_TXDESCHEAD 0x3810
#define E1000_REG_TXDESCTAIL 0x3818

#define E1000_REG_RXADDR     0x5400

#define RCTL_EN                         (1 << 1)    /* Receiver Enable */
#define RCTL_SBP                        (1 << 2)    /* Store Bad Packets */
#define RCTL_UPE                        (1 << 3)    /* Unicast Promiscuous Enabled */
#define RCTL_MPE                        (1 << 4)    /* Multicast Promiscuous Enabled */
#define RCTL_LPE                        (1 << 5)    /* Long Packet Reception Enable */
#define RCTL_LBM_NONE                   (0 << 6)    /* No Loopback */
#define RCTL_LBM_PHY                    (3 << 6)    /* PHY or external SerDesc loopback */
#define RTCL_RDMTS_HALF                 (0 << 8)    /* Free Buffer Threshold is 1/2 of RDLEN */
#define RTCL_RDMTS_QUARTER              (1 << 8)    /* Free Buffer Threshold is 1/4 of RDLEN */
#define RTCL_RDMTS_EIGHTH               (2 << 8)    /* Free Buffer Threshold is 1/8 of RDLEN */
#define RCTL_MO_36                      (0 << 12)   /* Multicast Offset - bits 47:36 */
#define RCTL_MO_35                      (1 << 12)   /* Multicast Offset - bits 46:35 */
#define RCTL_MO_34                      (2 << 12)   /* Multicast Offset - bits 45:34 */
#define RCTL_MO_32                      (3 << 12)   /* Multicast Offset - bits 43:32 */
#define RCTL_BAM                        (1 << 15)   /* Broadcast Accept Mode */
#define RCTL_VFE                        (1 << 18)   /* VLAN Filter Enable */
#define RCTL_CFIEN                      (1 << 19)   /* Canonical Form Indicator Enable */
#define RCTL_CFI                        (1 << 20)   /* Canonical Form Indicator Bit Value */
#define RCTL_DPF                        (1 << 22)   /* Discard Pause Frames */
#define RCTL_PMCF                       (1 << 23)   /* Pass MAC Control Frames */
#define RCTL_SECRC                      (1 << 26)   /* Strip Ethernet CRC */

#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))

#define TCTL_EN                         (1 << 1)    /* Transmit Enable */
#define TCTL_PSP                        (1 << 3)    /* Pad Short Packets */
#define TCTL_CT_SHIFT                   4           /* Collision Threshold */
#define TCTL_COLD_SHIFT                 12          /* Collision Distance */
#define TCTL_SWXOFF                     (1 << 22)   /* Software XOFF Transmission */
#define TCTL_RTLC                       (1 << 24)   /* Re-transmit on Late Collision */

#define CMD_EOP                         (1 << 0)    /* End of Packet */
#define CMD_IFCS                        (1 << 1)    /* Insert FCS */
#define CMD_IC                          (1 << 2)    /* Insert Checksum */
#define CMD_RS                          (1 << 3)    /* Report Status */
#define CMD_RPS                         (1 << 4)    /* Report Packet Sent */
#define CMD_VLE                         (1 << 6)    /* VLAN Packet Enable */
#define CMD_IDE                         (1 << 7)    /* Interrupt Delay Enable */

#define RX_STATUS_DD										(1 << 0)    /* Descriptor done */

#define STATUS_LINK_UP                  (1 << 1)    /* Link Up */

static void write_command(uint16_t addr, uint32_t val) {
	(*((volatile uint32_t*)(mem_base + addr))) = val;
}

static uint32_t read_command(uint16_t addr) {
	return *((volatile uint32_t*)(mem_base + addr));
}

static int eeprom_detect(void) {

	write_command(E1000_REG_EEPROM, 1);

	for (int i = 0; i < 100000 && !has_eeprom; ++i) {
		uint32_t val = read_command(E1000_REG_EEPROM);
		if (val & 0x10) has_eeprom = 1;
	}

	return 0;
}

static uint16_t eeprom_read(uint8_t addr) {
	uint32_t temp = 0;
	write_command(E1000_REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
	while (!((temp = read_command(E1000_REG_EEPROM)) & (1 << 4)));
	return (uint16_t)((temp >> 16) & 0xFFFF);
}


static void find_e1000(uint32_t device, uint16_t vendorid, uint16_t deviceid, void * extra) {
	if ((vendorid == 0x8086) && (deviceid == 0x100e || deviceid == 0x1004 || deviceid == 0x100f || deviceid == 0x10ea)) {
		*((uint32_t *)extra) = device;
	}
}

static void write_mac(void) {

	uint32_t low;
	uint32_t high;

	memcpy(&low, &e1000_mac[0], 4);
	memcpy(&high,&e1000_mac[4], 2);
	memset((uint8_t *)&high + 2, 0, 2);
	high |= 0x80000000;

	write_command(E1000_REG_RXADDR + 0, low);
	write_command(E1000_REG_RXADDR + 4, high);
}

static void read_mac(void) {
	if (has_eeprom) {
		for (int i = 0; i < 3; i++) {
			uint32_t part = eeprom_read(i);
			e1000_mac[2*i] = part & 0xFF;
			e1000_mac[2*i + 1] = (part >> 8) & 0xFF;
		}
	} else {
		uint8_t* mac_addr = (uint8_t*)(mem_base + E1000_REG_RXADDR);
		for (int i = 0; i < 6; ++i) {
			e1000_mac[i] = mac_addr[i];
		}
	}
}


// Receives a packet, returning the size of the packet or 0 if no packet was received
//  User is responsible for freeing the buffer that we will allocate
static size_t receive_packet(uint8_t** payload) {
	uint32_t rx_index = read_command(E1000_REG_RXDESCTAIL);
	if (rx_index == read_command(E1000_REG_RXDESCHEAD)) {
		// head == tail, so the queue is empty
		return 0;
	}

	rx_index = (rx_index + 1) % E1000_NUM_RX_DESC;
	uint32_t packetstatus = rx[rx_index].status;
	if (!(packetstatus & (RX_STATUS_DD))) {
		// The network card isn't done receiving this packet
		return 0;
	}
	// Normally, we would have to check if this is the end of the packet, but
	//  since we receive in chunks of 2048, an ethernet frame always fits in one chunk
	uint8_t* packet_address = (uint8_t*) rx_virt[rx_index];
	size_t size = (size_t) rx[rx_index].length;
	void* user_packet = alloc(size);
	memcpy(user_packet, packet_address, size);

	// Set the status to done
	rx[rx_index].status = 0;

	// Update the network card's tail
	write_command(E1000_REG_RXDESCTAIL, rx_index);
	*payload = user_packet;
	return size;
}

static void send_packet(uint8_t* payload, size_t payload_size) {
	uint32_t tx_index = read_command(E1000_REG_TXDESCTAIL);

	memcpy(tx_virt[tx_index], payload, payload_size);
	tx[tx_index].length = payload_size;
	// End Of Packet, let hardware generate checksum
	tx[tx_index].cmd = CMD_EOP | CMD_IFCS;
	tx[tx_index].status = 0;

	tx_index = (tx_index + 1) % E1000_NUM_TX_DESC;
	write_command(E1000_REG_TXDESCTAIL, tx_index);
}

static void init_rx(void) {
	// Set physical address of receive FIFO
	write_command(E1000_REG_RXDESCLO, rx_phys);
	write_command(E1000_REG_RXDESCHI, 0);

	write_command(E1000_REG_RXDESCLEN, E1000_NUM_RX_DESC * sizeof(struct rx_desc));

	// Initialize head and tail of receive FIFO
	write_command(E1000_REG_RXDESCHEAD, 0);
	write_command(E1000_REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);

	// Enable receiving, receive packets of up to 2048, allow receiving broadcast packets
	write_command(E1000_REG_RCTRL,
		RCTL_EN  | RCTL_BSIZE_2048 | RCTL_BAM |
		(read_command(E1000_REG_RCTRL)));

}

static void init_tx(void) {

	// Set physical address of transmit FIFO
	write_command(E1000_REG_TXDESCLO, tx_phys);
	write_command(E1000_REG_TXDESCHI, 0);

	write_command(E1000_REG_TXDESCLEN, E1000_NUM_TX_DESC * sizeof(struct tx_desc));

	// Initialize head and tail of transmit FIFO
	write_command(E1000_REG_TXDESCHEAD, 0);
	write_command(E1000_REG_TXDESCTAIL, 0);

	// Enable transmitting, Pad Short Packets
	write_command(E1000_REG_TCTRL,
		TCTL_EN |
		TCTL_PSP |
		read_command(E1000_REG_TCTRL));
}

static int e1000_init_main(void) {
	pci_scan(&find_e1000, -1, &e1000_device_pci);

	if (!e1000_device_pci) {
		terminal_writestring("No e1000 device found.");
		return 1;
	}

	mem_base  = pci_read_field(e1000_device_pci, PCI_BAR0, 4) & 0xFFFFFFF0;

	// TODO mark page as cache-disabled

	// TODO shrink network buffer size to RCTL_BSIZE_2048
	// TODO align to paragraph instead of to page

	// We don't do paging, so the virtual address = the physical address
	rx = valloc(sizeof(struct rx_desc) * E1000_NUM_RX_DESC + 16, 12);
	rx_phys = (uintptr_t) rx;
	for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
		// Allocate a 2048-sized piece of memory, aligned (so the last 4 bits are 0)
		rx_virt[i] = valloc(2048, 4);
		rx[i].addr = (uintptr_t) rx_virt[i];
		rx[i].status = 0;
	}

  tx = valloc(sizeof(struct tx_desc) * E1000_NUM_TX_DESC + 16, 12);
  tx_phys = (uintptr_t) tx;
	for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
		tx_virt[i] = valloc(2048, 4);
    tx[i].addr = (uintptr_t) tx_virt[i];
		tx[i].status = 0;
		tx[i].cmd = (1 << 0);
	}

	// Enable PCI bus mastering
	uint16_t command_reg = pci_read_field(e1000_device_pci, PCI_COMMAND, 2);
	command_reg |= (1 << 2);
	command_reg |= (1 << 0);
	pci_write_field(e1000_device_pci, PCI_COMMAND, 2, command_reg);

	eeprom_detect();

	terminal_writestring("EEPROM=");
	terminal_writeint(has_eeprom, 10);

	read_mac();
	terminal_writestring(" MAC = ");
	terminal_writeint(e1000_mac[0], 16);
	terminal_writeint(e1000_mac[1], 16);
	terminal_writeint(e1000_mac[2], 16);
	terminal_writeint(e1000_mac[3], 16);
	terminal_writeint(e1000_mac[4], 16);
	terminal_writeint(e1000_mac[5], 16);
	terminal_writestring("\n");
	write_mac();

	init_rx();
	init_tx();

	int networkstatus = read_command(E1000_REG_STATUS);

  terminal_writestring("Network is ");
	if (networkstatus) {
		terminal_writestring("up!\n");
	} else {
		terminal_writestring("down :/ \n");
	}

	return 0;
}

#endif // DRIVERS_NETWORKING_E1000_C
