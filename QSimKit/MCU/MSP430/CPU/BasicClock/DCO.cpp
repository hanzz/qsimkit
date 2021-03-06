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

#include "DCO.h"
#include "CPU/Variants/Variant.h"
#include "CPU/Memory/Memory.h"
#include "CPU/Interrupts/InterruptManager.h"
#include <iostream>
#include <math.h>

namespace MSP430 {

#define ADD_WATCHER(METHOD) \
	if (METHOD != 0) { m_mem->addWatcher(METHOD, this); }

DCO::DCO(Memory *mem, Variant *variant) : Oscillator("DCO"), m_mem(mem), m_variant(variant),
m_freq(1000000), m_step(1.0/1000000) {
	ADD_WATCHER(m_variant->getDCOCTL());
	ADD_WATCHER(m_variant->getBCSCTL1());

	reset();
}

DCO::~DCO() {

}

double DCO::getStep() {
	return m_step;
}

void DCO::reset() {
	if (m_variant->getDCOCTL() != 0) {
		m_mem->setByte(m_variant->getDCOCTL(), 0x60);
	}

	if (m_variant->getBCSCTL1() != 0) {
		m_mem->setByte(m_variant->getBCSCTL1(), 0x87);
	}
}

void DCO::handleMemoryChanged(::Memory *memory, uint16_t address) {
	int rsel = m_mem->getByte(m_variant->getBCSCTL1()) & 0x0f;
	int dco = (m_mem->getByte(m_variant->getDCOCTL()) >> 5) & 0x7;
	int mod = m_mem->getByte(m_variant->getDCOCTL()) & 0x1f;

	double freq = m_variant->getDCOZERO() * 1000000;

	if (dco != 0) {
		freq *= pow(m_variant->getSDCO(), dco);
	}

	if (rsel != 0) {
		freq *= pow(m_variant->getSRSEL(), rsel);
	}

	m_freq = freq;
	m_step = 1.0 / m_freq;
}

}
