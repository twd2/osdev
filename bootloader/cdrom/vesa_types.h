#ifndef _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_TYPES_H_
#define _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_TYPES_H_

#include <runtime/types.h>

typedef struct vesa_control_info
{
   char magic[4]; // "VESA"
   uint16_t version; // == 0x0300 for VBE 3.0
   uint16_t oem_string_offset;
   uint16_t oem_string_seg;
   uint32_t capabilities;
   uint16_t video_mode_offset;
   uint16_t video_mode_seg;
   uint16_t total_memory; // x 64KB
   uint8_t reserved[492];
} __attribute__((packed)) vesa_control_info_t;

typedef struct vesa_mode_info
{
  uint16_t attr;
  uint8_t win_a, win_b;
  uint16_t win_granularity;
  uint16_t win_size;
  uint16_t seg_a, seg_b;
  uint32_t real_mode_func_ptr;
  uint16_t pitch; // bytes per scanline
 
  uint16_t x_res, y_res; // width, height
  uint8_t w_char, y_char, planes, bpp, banks;
  uint8_t memory_model, bank_size, image_pages;
  uint8_t reserved0;
 
  uint8_t red_mask, red_position;
  uint8_t green_mask, green_position;
  uint8_t blue_mask, blue_position;
  uint8_t rsv_mask, rsv_position;
  uint8_t directcolor_attr;
 
  uint32_t base;  // framebuffer physical address
  uint32_t reserved1;
  uint16_t reserved2;
  uint8_t reserved[206];
} __attribute__((packed)) vesa_mode_info_t;

#endif // _WDOS_BOOTLOADER_CDROM_STAGE2_VESA_TYPES_H_
