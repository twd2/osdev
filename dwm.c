#include "dwm.h"

#include <runtime/types.h>
#include <asm.h>
#include <process.h>
#include <driver/vesa.h>
#include <syscall.h>

static uint8_t dwm_stack[0x1000];

uint32_t dwm_pid;

void init_dwm()
{
    dwm_pid = process_create_kernel("dwm", &dwm_entry, &dwm_stack[sizeof(dwm_stack)]);
    kprint_ok_fail("[KDEBUG] create windows manager process", dwm_pid != (uint32_t)-1);
}

void dwm_entry()
{
    if (!vesa_available)
    {
        process_set_priority(PROCESS_PRIORITY_MIN);
        kprint("[DWM] No GUI available :(\n");
        while (true)
        {
            sys_yield();
        }
    }
    process_set_priority(PROCESS_PRIORITY_MAX);
    kprint("[DWM] Inited, press F8 to enter GUI.\n");
    while (true)
    {
        if (tty_current_screen() != tty_select(7))
        {
            while (tty_current_screen() != tty_select(7))
            {
                sys_yield();
            }
            vesa_clear(0x00a8a8);
            //vesa_clear(0x6060ff);
        }
        kprint("[DWM] Rendering...\n");
        for (int i = 0; i < 26; ++i)
        {
            vesa_fill_char(100 + i * 9, 160, 'a' + i, 0xa8a8a8, 0);
        }
        for (int i = 0; i < 26; ++i)
        {
            vesa_fill_char(100 + i * 9, 176, 'A' + i, 0xa8a8a8, 0);
        }
        for (int i = 0; i < 26; ++i)
        {
            vesa_fill_char_transparent(100 + i * 9, 192, 'a' + i, 0);
        }
        for (int i = 0; i < 26; ++i)
        {
            vesa_fill_char_transparent(100 + i * 9, 208, 'A' + i, 0);
        }
        /*for (int y = 1; y <= 256; ++y)
        {
            for (int i = 0; i < 6; ++i)
            {
                vesa_fill_char_scale_transparent(i * (y / 2 + 1), 0, '1' + i, y, 0xa8a8a8);
            }
        }*/
        for (int i = 0; i < 0x100; ++i)
        {
            vesa_draw_vline(10 + i, 10, 30, 0xff0000 | i);
        }
        for (int i = 0; i < 0xff; ++i)
        {
            vesa_draw_vline(265 + i, 10, 30, 0xff00ff - (i << 16));
        }
        for (int i = 0; i < 0xff; ++i)
        {
            vesa_draw_vline(520 + i, 10, 30, 0x0000ff | (i << 8));
        }
        vesa_fill_rect(50, 50, 50, 50,   0xff0000);
        vesa_fill_rect(105, 50, 50, 50,  0x00ff00);
        vesa_fill_rect(160, 50, 50, 50,  0x0000ff);
        vesa_fill_rect(50, 105, 50, 50,  0x00ffff);
        vesa_fill_rect(105, 105, 50, 50, 0xff00ff);
        vesa_fill_rect(160, 105, 50, 50, 0xffff00);
        /*vesa_draw_hline(0, 570, 800, 0xffffff);
        vesa_draw_vline(0, 570, 29, 0xffffff);
        vesa_draw_hline(0, 599, 800, 0);
        vesa_draw_vline(799, 570, 30, 0);
        vesa_fill_rect(1, 571, 798, 28, 0x808080);*/
        vesa_draw_button_rect(0, 570, 800, 30, false);
        vesa_draw_button_rect(5, 575, 50, 20, false);
        vesa_draw_button_rect(744, 575, 50, 20, true);
        vesa_fill_string(9, 578, -1, "Start", 0);
        vesa_fill_string(748, 578, -1, "23:17", 0);
        vesa_fill_string(328, 284, -1, "Welcome to WDOS!", 0);
        vesa_fill_string(296, 300, -1, "GUI is not implemented.", 0);
        vesa_fill_string(287, 316, -1, "Press F1~F7 to enter TTY.", 0);
        while (tty_current_screen() == tty_select(7))
        {
            sys_yield();
        }
    }
}
