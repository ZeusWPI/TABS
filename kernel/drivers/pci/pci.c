#ifndef DRIVERS_PCI_PCI_C
#define DRIVERS_PCI_PCI_C

typedef void (*pci_func_t)(uint32_t device, uint16_t vendor_id, uint16_t device_id, void * extra);
void pci_scan_bus(pci_func_t f, int type, int bus, void * extra);

#define PCI_VENDOR_ID            0x00 // 2
#define PCI_DEVICE_ID            0x02 // 2
#define PCI_COMMAND              0x04 // 2
#define PCI_STATUS               0x06 // 2
#define PCI_REVISION_ID          0x08 // 1

#define PCI_PROG_IF              0x09 // 1
#define PCI_SUBCLASS             0x0a // 1
#define PCI_CLASS                0x0b // 1
#define PCI_CACHE_LINE_SIZE      0x0c // 1
#define PCI_LATENCY_TIMER        0x0d // 1
#define PCI_HEADER_TYPE          0x0e // 1
#define PCI_BIST                 0x0f // 1
#define PCI_BAR0                 0x10 // 4
#define PCI_BAR1                 0x14 // 4
#define PCI_BAR2                 0x18 // 4
#define PCI_BAR3                 0x1C // 4
#define PCI_BAR4                 0x20 // 4
#define PCI_BAR5                 0x24 // 4

#define PCI_INTERRUPT_LINE       0x3C // 1

#define PCI_SECONDARY_BUS        0x19 // 1

#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT   0xCFC

#define PCI_NONE 0xFFFF

typedef void (*pci_func_t)(uint32_t device, uint16_t vendor_id, uint16_t device_id, void * extra);

static inline int pci_extract_bus(uint32_t device) {
	return (uint8_t)((device >> 16));
}
static inline int pci_extract_slot(uint32_t device) {
	return (uint8_t)((device >> 8));
}
static inline int pci_extract_func(uint32_t device) {
	return (uint8_t)(device);
}

static inline uint32_t pci_get_addr(uint32_t device, int field) {
	return 0x80000000 | (pci_extract_bus(device) << 16) | (pci_extract_slot(device) << 11) | (pci_extract_func(device) << 8) | ((field) & 0xFC);
}

static inline uint32_t pci_box_device(int bus, int slot, int func) {
	return (uint32_t)((bus << 16) | (slot << 8) | func);
}


void pci_write_field(uint32_t device, int field, int size, uint32_t value) {
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));
	outl(PCI_VALUE_PORT, value);
}

uint32_t pci_read_field(uint32_t device, int field, int size) {
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));

	if (size == 4) {
		uint32_t t = inl(PCI_VALUE_PORT);
		return t;
	} else if (size == 2) {
		uint16_t t = inw(PCI_VALUE_PORT + (field & 2));
		return t;
	} else if (size == 1) {
		uint8_t t = inb(PCI_VALUE_PORT + (field & 3));
		return t;
	}
	return 0xFFFF;
}

uint16_t pci_find_type(uint32_t dev) {
	return (pci_read_field(dev, PCI_CLASS, 1) << 8) | pci_read_field(dev, PCI_SUBCLASS, 1);
}


void pci_scan_hit(pci_func_t f, uint32_t dev, void * extra) {
	int dev_vend = (int)pci_read_field(dev, PCI_VENDOR_ID, 2);
	int dev_dvid = (int)pci_read_field(dev, PCI_DEVICE_ID, 2);

	f(dev, dev_vend, dev_dvid, extra);
}

void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, func);
	if (type == -1 || type == pci_find_type(dev)) {
		pci_scan_hit(f, dev, extra);
	}
	if (pci_find_type(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(f, type, pci_read_field(dev, PCI_SECONDARY_BUS, 1), extra);
	}
}

void pci_scan_slot(pci_func_t f, int type, int bus, int slot, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, 0);
	if (pci_read_field(dev, PCI_VENDOR_ID, 2) == PCI_NONE) {
		return;
	}
	pci_scan_func(f, type, bus, slot, 0, extra);
	if (!pci_read_field(dev, PCI_HEADER_TYPE, 1)) {
		return;
	}
	for (int func = 1; func < 8; func++) {
		uint32_t dev = pci_box_device(bus, slot, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
			pci_scan_func(f, type, bus, slot, func, extra);
		}
	}
}

void pci_scan_bus(pci_func_t f, int type, int bus, void * extra) {
	for (int slot = 0; slot < 32; ++slot) {
		pci_scan_slot(f, type, bus, slot, extra);
	}
}

void pci_scan(pci_func_t f, int type, void * extra) {

	if ((pci_read_field(0, PCI_HEADER_TYPE, 1) & 0x80) == 0) {
		pci_scan_bus(f,type,0,extra);
		return;
	}

	for (int func = 0; func < 8; ++func) {
		uint32_t dev = pci_box_device(0, 0, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
			pci_scan_bus(f, type, func, extra);
		} else {
			break;
		}
	}
}

static void find_isa_bridge(uint32_t device, uint16_t vendorid, uint16_t deviceid, void * extra) {
	if (vendorid == 0x8086 && (deviceid == 0x7000 || deviceid == 0x7110)) {
		*((uint32_t *)extra) = device;
	}
}
static uint32_t pci_isa = 0;
static uint8_t pci_remaps[4] = {0};
void pci_remap(void) {
	pci_scan(&find_isa_bridge, -1, &pci_isa);
	if (pci_isa) {
		for (int i = 0; i < 4; ++i) {
			pci_remaps[i] = pci_read_field(pci_isa, 0x60+i, 1);
			if (pci_remaps[i] == 0x80) {
				pci_remaps[i] = 10 + (i%1);
			}
		}
		uint32_t out = 0;
		memcpy(&out, &pci_remaps, 4);
		pci_write_field(pci_isa, 0x60, 4, out);
	}
}

int pci_get_interrupt(uint32_t device) {

	if (pci_isa) {
		uint32_t irq_pin = pci_read_field(device, 0x3D, 1);
		if (irq_pin == 0) {
			return pci_read_field(device, PCI_INTERRUPT_LINE, 1);
		}
		int pirq = (irq_pin + pci_extract_slot(device) - 2) % 4;
		int int_line = pci_read_field(device, PCI_INTERRUPT_LINE, 1);
		if (pci_remaps[pirq] >= 0x80) {
			if (int_line == 0xFF) {
				int_line = 10;
				pci_write_field(device, PCI_INTERRUPT_LINE, 1, int_line);
			}
			pci_remaps[pirq] = int_line;
			uint32_t out = 0;
			memcpy(&out, &pci_remaps, 4);
			pci_write_field(pci_isa, 0x60, 4, out);
			return int_line;
		}
		pci_write_field(device, PCI_INTERRUPT_LINE, 1, pci_remaps[pirq]);
		return pci_remaps[pirq];
	} else {
		return pci_read_field(device, PCI_INTERRUPT_LINE, 1);
	}
}

#endif // DRIVERS_PCI_PCI_C
