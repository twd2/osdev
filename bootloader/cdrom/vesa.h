#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_H_

#include <runtime/types.h>

#include "vesa_types.h"

#define VESA_MAGIC "VESA"
#define VESA_MAGIC_LENGTH 4
#define VESA_MODE_INVALID 0xffff
#define VESA_MODE_TERMINATOR 0xffff
#define VESA_MODE_LINEAR_FRAMEBUFFER 0x4000
#define VESA_MODE_ATTR_GRAPHICS 0x10
#define VESA_MODE_ATTR_LINEAR_FRAMEBUFFER 0x80
#define VESA_MODE_MEMORY_MODEL_DIRECTCOLOR 0x6

uint32_t mode_diff(uint16_t target_width, uint16_t target_height, uint8_t target_depth,
                   uint16_t width, uint16_t height, uint8_t depth);

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_H_
