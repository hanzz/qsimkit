/**
 * QSimKit - MSP430 simulator
 * Copyright (C) 2013 Jan "HanzZ" Kaluza (hanzz.k@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

class Memory;

class MemoryWatcher {
	public:
		typedef enum { Read, Write, ReadWrite } Mode;

		virtual void handleMemoryChanged(Memory *memory, uint16_t address) = 0;

		virtual void handleMemoryRead(Memory *memory, uint16_t address, uint16_t &value) {}
		virtual void handleMemoryRead(Memory *memory, uint16_t address, uint8_t &value) {}
};

class Memory {
	public:

		virtual uint16_t get(uint16_t address) = 0;
		virtual uint16_t getBigEndian(uint16_t address, bool watchers = true) = 0;
		virtual void set(uint16_t address, uint16_t value) = 0;
		virtual void setBigEndian(uint16_t address, uint16_t value, bool watchers = true) = 0;

		virtual uint8_t getByte(uint16_t address, bool watchers = true) = 0;
		virtual void setByte(uint16_t address, uint8_t value, bool watchers = true) = 0;
		virtual bool isBitSet(uint16_t address, uint16_t bit) = 0;
		virtual void setBit(uint16_t address, uint16_t bit, bool value) = 0;
		
		virtual void addWatcher(uint16_t address, MemoryWatcher *watcher, MemoryWatcher::Mode mode = MemoryWatcher::Write) = 0;
		virtual void removeWatcher(uint16_t address, MemoryWatcher *watcher, MemoryWatcher::Mode mode = MemoryWatcher::ReadWrite) = 0;
};

