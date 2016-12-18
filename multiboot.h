/* multiboot.h - the header for Multiboot */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Macros.  */

/* The magic number for the Multiboot header.  */
#define MULTIBOOT_HEADER_MAGIC    0x1BADB002

/* The flags for the Multiboot header.  */
#ifdef __ELF__
# define MULTIBOOT_HEADER_FLAGS   0x00000003
#else
# define MULTIBOOT_HEADER_FLAGS   0x00010003
#endif

/* The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

/* The size of our stack (16KB).  */
#define STACK_SIZE      0x4000

/* C symbol format. HAVE_ASM_USCORE is defined by configure.  */
#ifdef HAVE_ASM_USCORE
# define EXT_C(sym)     _ ## sym
#else
# define EXT_C(sym)     sym
#endif

#ifndef ASM
/* Do not include here in boot.S.  */

/* Types.  */
#include <runtime/types.h>

/* The Multiboot header.  */
typedef struct multiboot_header
{
  u32 magic;
  u32 flags;
  u32 checksum;
  u32 header_addr;
  u32 load_addr;
  u32 load_end_addr;
  u32 bss_end_addr;
  u32 entry_addr;
} multiboot_header_t;

/* The symbol table for a.out.  */
typedef struct aout_symbol_table
{
  u32 tabsize;
  u32 strsize;
  u32 addr;
  u32 reserved;
} aout_symbol_table_t;

/* The section header table for ELF.  */
typedef struct elf_section_header_table
{
  u32 num;
  u32 size;
  u32 addr;
  u32 shndx;
} elf_section_header_table_t;

/* The Multiboot information.  */
typedef struct multiboot_info
{
  u32 flags;
  u32 mem_lower;
  u32 mem_upper;
  u32 boot_device;
  u32 cmdline;
  u32 mods_count;
  u32 mods_addr;
  union
  {
    aout_symbol_table_t aout_sym;
    elf_section_header_table_t elf_sec;
  } u;
  u32 mmap_length;
  u32 mmap_addr;
} multiboot_info_t;

/* The module structure.  */
typedef struct module
{
  u32 mod_start;
  u32 mod_end;
  u32 string;
  u32 reserved;
} module_t;

/* The memory map. Be careful that the offset 0 is base_addr_low
   but no size.  */
typedef struct memory_map
{
  u32 size;
  u32 base_addr_low;
  u32 base_addr_high;
  u32 length_low;
  u32 length_high;
  u32 type;
} memory_map_t;

#endif /* ! ASM */
