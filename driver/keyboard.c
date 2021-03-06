#include "keyboard.h"
#include <io.h>
#include <tty.h>

static uint8_t left_shift_down_count = 0;
static uint8_t right_shift_down_count = 0;
static uint8_t left_ctrl_down_count = 0;
static uint8_t right_ctrl_down_count = 0;
static uint8_t left_alt_down_count = 0;
static uint8_t right_alt_down_count = 0;

void init_keyboard()
{
    register_irq_handler(IRQ_KEYBOARD, keyboard_irq_handler);
    keyboard_read(); // eat
}

inline uint8_t keyboard_read()
{
    uint8_t b = inb(KEYBOARD_BUFFER);
    io_delay();
    return b;
}

void keyboard_irq_handler(uint8_t irq, interrupt_frame_t *frame)
{
    uint8_t scancode = keyboard_read();
    /*tty_set_current(default_tty);
    kprint("[KEYBOARD] ");
    kprint_hex(scancode);
    kprint("\n");
    tty_set_current(NULL);*/
    // TODO: extend code
    switch (scancode)
    {
    case 0x1d: // ctrl make
        ++left_ctrl_down_count;
        break;
    case 0x9d: // ctrl break
        --left_ctrl_down_count;
        break;
    case 0x2a: // shift make
        ++left_shift_down_count;
        break;
    case 0xaa: // shift break
        --left_shift_down_count;
        break;
    case 0x38: // alt make
        ++left_alt_down_count;
        break;
    case 0xb8: // alt break
        --left_alt_down_count;
        break;
    case 0x36: // right shift make
        ++right_shift_down_count;
        break;
    case 0xb6: // right shift break
        --right_shift_down_count;
        break;
    case 0x53: // delete make
        if (left_ctrl_down_count && left_alt_down_count)
        {
            asm("ud2");
        }
        break;
    default:
        if (0x3b <= scancode && scancode <= 0x3b + 7)
        {
            tty_switch(default_tty + scancode - 0x3b);
        }
        break;
    }
}
