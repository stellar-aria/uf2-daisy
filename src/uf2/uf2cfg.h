#include "boards.h"
// #include "dfu_types.h"

// Virtual disk size: just under 32MB
#define CFG_UF2_NUM_BLOCKS 0x10109

// Board-specific ID for board-specific Application
#if defined(USB_DESC_VID) && defined(USB_DESC_UF2_PID) && USB_DESC_VID && USB_DESC_UF2_PID
#define CFG_UF2_BOARD_APP_ID ((USB_DESC_VID << 16) | USB_DESC_UF2_PID)
#endif

// Family ID and size for updating generic Application
#define CFG_UF2_FAMILY_ID  0x6db66082
#define CFG_UF2_FLASH_SIZE (8 * 1024 * 1024) // 8 MB

// Application Address Space
#define USER_FLASH_START 0x90000000 // skip MBR included in SD hex
#define USER_FLASH_END   (0x90000000 + 0x800000)

// Address where new bootloader is written before activation (skip application data)
#define BOOTLOADER_ADDR_NEW_RECIEVED (USER_FLASH_END - DFU_BL_IMAGE_MAX_SIZE)
