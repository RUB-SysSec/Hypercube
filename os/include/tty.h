/*
 * HyperCube OS
 * (c) Sergej Schumilo, 2019 <sergej@schumilo.de> 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */ 

#pragma once

#include "system.h"
#include "vga.h"
#include "../../config.h"

#ifdef SHARKOS
#define DEFAULT_COLOR vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE)
#else
#define DEFAULT_COLOR vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_LIGHT_BLUE)
#endif

#define SUCCESS_COLOR vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_LIGHT_BLUE)
#define FAIL_COLOR vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_LIGHT_BLUE)
#define CRASH_COLOR vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED)

#define printf(...) terminal_printf(__VA_ARGS__);

#ifdef DEBUG_PRINTF
#define debug_printf(...) do { printf(__VA_ARGS__); } while (0)
#else
# define debug_printf(...) do { if((0))(void)printf(__VA_ARGS__); } while (0)
#endif

void terminal_initialize(uint8_t default_terminal_color);

void terminal_setcolor(uint8_t color);
void terminal_setpos(size_t column, size_t row);

void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_force_writestring(const char* data);
void terminal_writestring(const char* data);
int terminal_printf(const char* fmt, ...);

void terminal_drawbmp(uint8_t* data);
void enable_printf(void);
void disable_printf(void);