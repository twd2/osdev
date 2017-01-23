#ifndef _WDOS_KERNEL_DRIVER_VESA_H_
#define _WDOS_KERNEL_DRIVER_VESA_H_

#include <runtime/types.h>
#include <multiboot.h>
#include <tty.h>

#define VESA_MAGIC "VESA"
#define VESA_MAGIC_LENGTH 4
#define VESA_MODE_INVALID 0xffff
#define VESA_MODE_TERMINATOR 0xffff
#define VESA_MODE_LINEAR_FRAMEBUFFER 0x4000
#define VESA_MODE_ATTR_GRAPHICS 0x10
#define VESA_MODE_ATTR_LINEAR_FRAMEBUFFER 0x80
#define VESA_MODE_MEMORY_MODEL_DIRECTCOLOR 0x6

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

typedef uint32_t color_t;
#define MKCOLOR(r, g, b) (((color_t)((r) & 0xff) << 16) | ((color_t)((g) & 0xff) << 8) | \
                          ((color_t)((b) & 0xff)))
#define COLOR_BLACK MKCOLOR(0, 0, 0)
#define COLOR_WHITE MKCOLOR(0xff, 0xff, 0xff)
#define COLOR_R(color) (((color) >> 16) & 0xff)
#define COLOR_G(color) (((color) >> 8) & 0xff)
#define COLOR_B(color) (((color)) & 0xff)
#define COLOR_RGB(color, r, g, b) \
    do \
    { \
        r = COLOR_R(color); \
        g = COLOR_G(color); \
        b = COLOR_B(color); \
    } \
    while (0)
#define VESA_CHAR_WIDTH (VGA_FONT_WIDTH + 1)
#define VESA_CHAR_HEIGHT (VGA_FONT_HEIGHT)

extern bool vesa_available;

void init_vesa(multiboot_info_t *mb_info);
uint32_t vesa_device_color(color_t color);
void vesa_clear(color_t color);
void vesa_fill_char(size_t x0, size_t y0, uint8_t ch, color_t fc, color_t bc);
void vesa_fill_char_scale(size_t x0, size_t y0, uint8_t ch,
                         size_t scale, color_t fc, color_t bc);
void vesa_fill_char_transparent(size_t x0, size_t y0, uint8_t ch, color_t fc);
void vesa_fill_char_scale_transparent(size_t x0, size_t y0, uint8_t ch,
                                      size_t scale, color_t fc);
void vesa_fill_rect(size_t x0, size_t y0, size_t width, size_t height, color_t color);
void vesa_draw_hline(size_t x0, size_t y0, size_t width, color_t color);
void vesa_draw_vline(size_t x0, size_t y0, size_t height, color_t color);
void vesa_draw_button_rect(size_t x0, size_t y0, size_t width, size_t height, bool pushed);
void vesa_fill_string(size_t x0, size_t y0, size_t width, const char *str, color_t fc);

// for tty driver
void vesa_init_tty_color();
void vesa_tty_init(tty_t *tty, size_t i);
void vesa_tty_rerender(tty_t *tty);
void vesa_tty_switch_handler(tty_t *tty);
void vesa_tty_update_cursor_location(tty_t *tty);
void vesa_tty_set_char(tty_t *tty, size_t x, size_t y, char ch, tty_color_t color);
const tty_driver_t *vesa_get_tty_driver();

#endif // _WDOS_KERNEL_DRIVER_VESA_H_
