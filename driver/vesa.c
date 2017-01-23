#include "vesa.h"

#include <mm/mm.h>
#include <stdlib/memory.h>
#include <driver/vga_font.h>
#include <asm.h>

bool vesa_available = false;

vesa_control_info_t vesa_control_info;
uint16_t vesa_mode;
vesa_mode_info_t vesa_mode_info;
void *vesa_framebuffer_va;

static uint32_t vesa_tty_color[16];

static spinlock_t vesa_lock;
static volatile uint32_t vesa_revision = 0;

static tty_driver_t vesa_tty_driver =
{
    .width = 0, // filled by init_vesa()
    .height = 0, // filled by init_vesa()
    .init = &vesa_tty_init,
    .rerender = &vesa_tty_rerender,
    .switch_handler = &vesa_tty_switch_handler,
    .update_cursor_location = &vesa_tty_update_cursor_location,
    .set_char = &vesa_tty_set_char
};

void init_vesa(multiboot_info_t *mb_info)
{
    if (mb_info->flags & MULTIBOOT_INFO_VIDEO_INFO)
    {
        vesa_control_info_t *mb_control_info =
            (vesa_control_info_t *)__VA((void *)mb_info->vbe_control_info);
        memcpy(&vesa_control_info, mb_control_info, sizeof(vesa_control_info_t));

        vesa_mode = mb_info->vbe_mode;

        vesa_mode_info_t *mb_mode_info =
            (vesa_mode_info_t *)__VA((void *)mb_info->vbe_mode_info);
        memcpy(&vesa_mode_info, mb_mode_info, sizeof(vesa_mode_info_t));

        vesa_available =
            (vesa_mode_info.attr & VESA_MODE_ATTR_LINEAR_FRAMEBUFFER) &&
            (vesa_mode_info.attr & VESA_MODE_ATTR_GRAPHICS) &&
            vesa_mode_info.memory_model == VESA_MODE_MEMORY_MODEL_DIRECTCOLOR;

        // TODO: paging: if vesa_available, map vesa_mode_info.base
        vesa_framebuffer_va = (void *)vesa_mode_info.base;

        vesa_clear(COLOR_BLACK);

        // for tty driver
        vesa_init_tty_color();
        vesa_tty_driver.width = vesa_mode_info.x_res / VESA_CHAR_WIDTH;
        vesa_tty_driver.height = vesa_mode_info.y_res / VESA_CHAR_HEIGHT;

        spinlock_init(&vesa_lock);
    }
    else
    {
        vesa_available = false;
    }
}

inline uint32_t vesa_device_color(color_t color)
{
    uint32_t r, g, b;
    COLOR_RGB(color, r, g, b);
    r = ((r << vesa_mode_info.red_mask) >> 8) << vesa_mode_info.red_position;
    g = ((g << vesa_mode_info.green_mask) >> 8) << vesa_mode_info.green_position;
    b = ((b << vesa_mode_info.blue_mask) >> 8) << vesa_mode_info.blue_position;
    return r | g | b;
}

#define OFFSET(fb, x0, y0) \
    ((uint8_t *)fb + vesa_mode_info.pitch * (y0) + vesa_mode_info.bpp * (x0) / 8)

#define SET_COLOR(lfb, color) \
    do \
    { \
        if (vesa_mode_info.bpp == 32) \
        { \
            uint32_t *lfb32 = (uint32_t *)lfb; \
            *lfb32 = color; \
            lfb += 4; \
        } \
        else if (vesa_mode_info.bpp == 24) \
        { \
            uint32_t *lfb32 = (uint32_t *)lfb; \
            *lfb32 = (*lfb32 & 0xff000000) | (color & 0xffffff); \
            lfb += 3; \
        } \
        else /* 15, 16 */ \
        { \
            uint16_t *lfb16 = (uint16_t *)lfb; \
            *lfb16 = (uint16_t)(color & 0xffff); \
            lfb += 2; \
        } \
    } while (0)

#define KEEP_COLOR(lfb) \
    do \
    { \
        lfb += vesa_mode_info.bpp >> 3; \
    } while (0)

#define RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, width, height, x, y) \
    for (size_t y = 0; y < (height); ++y) \
    { \
        uint8_t *lfb = fb; \
        for (size_t x = 0; x < (width); ++x) \
        {

#define RECTANGLE_FOR_EACH_PIXEL_END() \
        } \
        fb += vesa_mode_info.pitch; \
    }

void vesa_clear(color_t color)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = vesa_framebuffer_va;
    uint32_t device_color = vesa_device_color(color);

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, vesa_mode_info.x_res, vesa_mode_info.y_res,
                                   x, y)
        SET_COLOR(lfb, device_color);
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_fill_char(size_t x0, size_t y0, uint8_t ch, color_t fc, color_t bc)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint8_t *font = vga_font_get(ch);
    uint32_t device_fc = vesa_device_color(fc),
             device_bc = vesa_device_color(bc);

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, VESA_CHAR_WIDTH, VESA_CHAR_HEIGHT, x, y)
        uint32_t c;
        if (x < VGA_FONT_WIDTH && y < VGA_FONT_HEIGHT && ((font[y] << x) & 0x80)) // MSB, left
        {
            c = device_fc;
        }
        else
        {
            c = device_bc;
        }
        SET_COLOR(lfb, c);
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_fill_char_scale(size_t x0, size_t y0, uint8_t ch,
                                 size_t scale, color_t fc, color_t bc)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint8_t *font = vga_font_get(ch);
    uint32_t device_fc = vesa_device_color(fc),
             device_bc = vesa_device_color(bc);
    size_t char_width = scale / 2,
           char_height = scale;

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, char_width, char_height, x, y)
        uint32_t c;
        // MSB, left
        if ((font[y * VGA_FONT_HEIGHT / scale] << (x * 2 * VGA_FONT_WIDTH / scale)) & 0x80)
        {
            c = device_fc;
        }
        else
        {
            c = device_bc;
        }
        SET_COLOR(lfb, c);
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_fill_char_transparent(size_t x0, size_t y0, uint8_t ch, color_t fc)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint8_t *font = vga_font_get(ch);
    uint32_t device_fc = vesa_device_color(fc);

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, VGA_FONT_WIDTH, VGA_FONT_HEIGHT, x, y)
        if (x < VGA_FONT_WIDTH && y < VGA_FONT_HEIGHT && ((font[y] << x) & 0x80)) // MSB, left
        {
            SET_COLOR(lfb, device_fc);
        }
        else
        {
            KEEP_COLOR(lfb);
        }
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_fill_char_scale_transparent(size_t x0, size_t y0, uint8_t ch,
                                             size_t scale, color_t fc)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint8_t *font = vga_font_get(ch);
    uint32_t device_fc = vesa_device_color(fc);
    size_t char_width = scale / 2,
           char_height = scale;

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, char_width, char_height, x, y)
        // MSB, left
        if ((font[y * VGA_FONT_HEIGHT / scale] << (x * 2 * VGA_FONT_WIDTH / scale)) & 0x80)
        {
            SET_COLOR(lfb, device_fc);
        }
        else
        {
            KEEP_COLOR(lfb);
        }
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_fill_rect(size_t x0, size_t y0, size_t width, size_t height, color_t color)
{
    if (!vesa_available)
    {
        return;
    }
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint32_t device_color = vesa_device_color(color);

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, width, height, x, y)
        SET_COLOR(lfb, device_color);
    RECTANGLE_FOR_EACH_PIXEL_END()
}

inline void vesa_draw_hline(size_t x0, size_t y0, size_t width, color_t color)
{
    vesa_fill_rect(x0, y0, width, 1, color);
}
inline void vesa_draw_vline(size_t x0, size_t y0, size_t height, color_t color)
{
    vesa_fill_rect(x0, y0, 1, height, color);
}

void vesa_draw_button_rect(size_t x0, size_t y0, size_t width, size_t height, bool pushed)
{
    if (!pushed)
    {
        vesa_draw_hline(x0, y0, width, 0xe0e0e0);
        vesa_draw_vline(x0, y0, height - 1, 0xe0e0e0);
        vesa_draw_hline(x0, y0 + height - 1, width, 0x303030);
        vesa_draw_vline(x0 + width - 1, y0, height, 0x303030);
        vesa_fill_rect(x0 + 1, y0 + 1, width - 2, height - 2, 0x909090);
    }
    else
    {
        vesa_draw_hline(x0, y0, width, 0x303030);
        vesa_draw_vline(x0, y0, height - 1, 0x303030);
        vesa_draw_hline(x0, y0 + height - 1, width, 0xe0e0e0);
        vesa_draw_vline(x0 + width - 1, y0, height, 0xe0e0e0);
        vesa_fill_rect(x0 + 1, y0 + 1, width - 2, height - 2, 0x808080);
    }
}

void vesa_fill_string(size_t x0, size_t y0, size_t width, const char *str, color_t fc)
{
    if (width != (size_t)-1)
    {
        for (int i = 0; str[i] != '\0'; ++i)
        {
            vesa_fill_char_transparent(x0 + i % width * 9, y0 + i / width * 16,
                                       str[i], fc);
        }
    }
    else
    {
        for (int i = 0; str[i] != '\0'; ++i)
        {
            vesa_fill_char_transparent(x0 + i * 9, y0, str[i], fc);
        }
    }
}

void vesa_init_tty_color()
{
    vesa_tty_color[TTY_COLOR_BLACK] = vesa_device_color(MKCOLOR(0, 0, 0));
    vesa_tty_color[TTY_COLOR_BLUE] = vesa_device_color(MKCOLOR(0, 0, 128));
    vesa_tty_color[TTY_COLOR_GREEN] = vesa_device_color(MKCOLOR(0, 128, 0));
    vesa_tty_color[TTY_COLOR_CYAN] = vesa_device_color(MKCOLOR(0, 128, 128));
    vesa_tty_color[TTY_COLOR_RED] = vesa_device_color(MKCOLOR(128, 0, 0));
    vesa_tty_color[TTY_COLOR_MAGENTA] = vesa_device_color(MKCOLOR(128, 0, 128));
    vesa_tty_color[TTY_COLOR_BROWN] = vesa_device_color(MKCOLOR(128, 128, 0));
    vesa_tty_color[TTY_COLOR_LIGHTGREY] = vesa_device_color(MKCOLOR(168, 168, 168));
    vesa_tty_color[TTY_COLOR_DARKGREY] = vesa_device_color(MKCOLOR(128, 128, 128));
    vesa_tty_color[TTY_COLOR_LIGHTBLUE] = vesa_device_color(MKCOLOR(0, 0, 255));
    vesa_tty_color[TTY_COLOR_LIGHTGREEN] = vesa_device_color(MKCOLOR(0, 255, 0));
    vesa_tty_color[TTY_COLOR_LIGHTCYAN] = vesa_device_color(MKCOLOR(0, 255, 255));
    vesa_tty_color[TTY_COLOR_LIGHTRED] = vesa_device_color(MKCOLOR(255, 0, 0));
    vesa_tty_color[TTY_COLOR_LIGHTMAGENTA] = vesa_device_color(MKCOLOR(255, 0, 255));
    vesa_tty_color[TTY_COLOR_LIGHTBROWN] = vesa_device_color(MKCOLOR(255, 255, 0));
    vesa_tty_color[TTY_COLOR_WHITE] = vesa_device_color(MKCOLOR(255, 255, 255));
}

void vesa_tty_init(tty_t *tty, size_t i)
{
    // allocate colored chars buffer
    tty->mem = (tty_colored_char_t *)mm_palloc(vesa_tty_driver.width *
                                               vesa_tty_driver.height *
                                               sizeof(tty_colored_char_t));
}

void vesa_tty_rerender(tty_t *tty)
{
    // inc and get?
    ++vesa_revision;
    uint32_t current_revision = vesa_revision;

    uint8_t *fb = (uint8_t *)vesa_framebuffer_va;

    // clear
    /*uint32_t device_color = vesa_device_color(COLOR_BLACK);
    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, vesa_mode_info.x_res, vesa_mode_info.y_res,
                                   x, y)
        if (vesa_revision != current_revision)
        {
            return;
        }
        SET_COLOR(lfb, device_color);
    RECTANGLE_FOR_EACH_PIXEL_END()*/

    // reset
    fb = (uint8_t *)vesa_framebuffer_va;
    //size_t width = vesa_tty_driver.width * VESA_CHAR_WIDTH,
    //       height = vesa_tty_driver.height * VESA_CHAR_HEIGHT;
    tty_colored_char_t *lcch = tty->mem;
    for (size_t y = 0; y < vesa_tty_driver.height; ++y)
    {
        for (size_t y_font = 0; y_font < VESA_CHAR_HEIGHT; ++y_font)
        {
            uint8_t *lfb = fb;
            tty_colored_char_t *cch = lcch;
            for (size_t x = 0; x < vesa_tty_driver.width; ++x)
            {
                tty_color_t color = *cch >> 8;
                char ch = *cch & 0xff;
                uint8_t *font = vga_font_get(ch);
                ++cch;
                for (size_t x_font = 0; x_font < VESA_CHAR_WIDTH; ++x_font)
                {
                    if (vesa_revision != current_revision)
                    {
                        return;
                    }
                    uint32_t c;
                    // MSB, left
                    if (x_font < VGA_FONT_WIDTH && y_font < VGA_FONT_HEIGHT &&
                        ((font[y_font] << x_font) & 0x80))
                    {
                        c = vesa_tty_color[color & 0xf];
                    }
                    else
                    {
                        c = vesa_tty_color[color >> 4];
                    }
                    SET_COLOR(lfb, c);
                }
            }
            fb += vesa_mode_info.pitch;
        }
        lcch += vesa_tty_driver.width;
    }
}

void vesa_tty_switch_handler(tty_t *tty)
{
    vesa_tty_rerender(tty);
}

void vesa_tty_update_cursor_location(tty_t *tty)
{
    // TODO
}

void vesa_tty_set_char(tty_t *tty, size_t x, size_t y, char ch, tty_color_t color)
{
    size_t x0 = x * VESA_CHAR_WIDTH, y0 = y * VESA_CHAR_HEIGHT;
    uint8_t *fb = OFFSET(vesa_framebuffer_va, x0, y0);
    uint8_t *font = vga_font_get(ch);
    uint32_t device_fc = vesa_tty_color[color & 0xf],
             device_bc = vesa_tty_color[color >> 4];

    RECTANGLE_FOR_EACH_PIXEL_BEGIN(fb, lfb, VESA_CHAR_WIDTH, VESA_CHAR_HEIGHT, x_font, y_font)
        uint32_t c;
        if (x_font < VGA_FONT_WIDTH && y_font < VGA_FONT_HEIGHT &&
            (font[y_font] << x_font) & 0x80) // MSB, left
        {
            c = device_fc;
        }
        else
        {
            c = device_bc;
        }
        SET_COLOR(lfb, c);
    RECTANGLE_FOR_EACH_PIXEL_END()
}

const tty_driver_t *vesa_get_tty_driver()
{
    return (const tty_driver_t *)&vesa_tty_driver;
}
