# COB External XSPI Flash Layout

The device uses external XSPI NOR flash (`EXTMEMORY_2`, `hxspi2`) for persistent
configuration and future firmware/update storage.

## Reserved Regions

| Region | Address | Size | Purpose |
| --- | --- | ---: | --- |
| Configuration | Last erase sector of flash | One erase sector | Persistent COB configuration. |
| Firmware/update area | Start of flash up to configuration sector | TBD | Future bootloader and main firmware update storage. |

The configuration sector is selected at runtime from SFDP flash information:

1. Read flash size and erase types via `EXTMEM_GetInfo(EXTMEMORY_2, ...)`.
2. Select the smallest available erase size.
3. Place the configuration record at `flash_size - selected_erase_size`.

Firmware and bootloader images must not use the last erase sector.

## Configuration Record

The configuration record starts with:

| Field | Type | Description |
| --- | --- | --- |
| `magic` | `uint32_t` | `0x43464743` (`CFGC`). |
| `version` | `uint16_t` | Record format version. Current: `1`. |
| `length` | `uint16_t` | Size of `COB_Config_t`. |
| `crc32` | `uint32_t` | CRC32 over `COB_Config_t`. |
| `reserved` | `uint32_t` | Reserved, erased value. |
| `config` | `COB_Config_t` | Persistent configuration payload. |

## Current Configuration Sections

### Addresses

| Field | Type | Default |
| --- | --- | --- |
| IP address | `uint8_t[4]` | `192.168.1.188` |
| MAC address | `uint8_t[6]` | `02:FD:<UID[31:24]>:<UID[23:16]>:<UID[15:8]>:<UID[7:0]>` |

The default MAC address uses the low 32 bits returned by `HAL_GetUIDw0()`.
