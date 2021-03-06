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

#include <QPainter>
#include <map>
#include "Peripherals/Peripheral.h"
#include "ConnectionManager.h"
#include <QDebug>

class ConnectionNode : public Peripheral
{
	Q_OBJECT

	public:
		ConnectionNode();
		virtual ~ConnectionNode();

		void paint(QWidget *screen);

		void setConnection(int pin, Connection *c) {
			qDebug() << "adding" << pin;
			m_conns[pin] = c;
		}

		void removeConnection(int pin) {
			m_conns.erase(pin);
		}

// 		void getAllConnectedObjects(std::vector<ScreenObject *> &objects);

		Connection *getConnection(int pin) {
			if (m_conns.find(pin) == m_conns.end()) {
				return 0;
			}
			return m_conns[pin];
		}

		bool isUseless() {
			return m_conns.size() < 3;
		}

		PinList &getPins() {
			return m_pins;
		}

		const QStringList &getOptions() {
			return m_options;
		}

		void executeOption(int) {}

		void internalTransition();

		void externalEvent(double t, const SimulationEventList &);

		void output(SimulationEventList &output);

		double timeAdvance();

		void reset();

	private:
		PinList m_pins;
		std::map<int, Connection *> m_conns;
		QStringList m_options;
		SimulationEventList m_output;
		double m_advance;

};

