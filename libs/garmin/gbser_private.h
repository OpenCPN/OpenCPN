/*
    Serial interface - private header for gbser*.c

    Copyright (C) 2006-2014  Robert Lipe, robertlipe+source@gpsbabel.org

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#define MYMAGIC 0x91827364
#define BUFSIZE 512

void gbser__db(int l, const char* msg, ...);
int gbser__fill_buffer(void* h, unsigned want, unsigned* ms);
unsigned gbser__read_buffer(void* handle, void** buf, unsigned* len);
