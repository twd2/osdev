#include "keyboard.h"
#include <io.h>
#include <kstdio.h>

static uint8_t left_shift_down_count = 0;
static uint8_t right_shift_down_count = 0;
static uint8_t left_ctrl_down_count = 0;
static uint8_t right_ctrl_down_count = 0;
static uint8_t left_alt_down_count = 0;
static uint8_t right_alt_down_count = 0;

void init_keyboard()
{
    register_irq_handler(IRQ_KEYBOARD, keyboard_irq_handler);
}

inline uint8_t keyboard_read()
{
    return inb(KEYBOARD_BUFFER);
}

void keyboard_irq_handler(uint8_t irq, interrupt_frame_t *frame)
{
    uint8_t scancode = keyboard_read();
    kprint_hex(scancode);
    kprint(" ");
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
        break;
    }
}
