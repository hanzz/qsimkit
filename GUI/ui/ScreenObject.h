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

typedef struct {
	QRect rect;
	QString name;
	bool high;
} Pin;

class ScreenObject : public QObject
{
	Q_OBJECT

	public:
		ScreenObject();
		virtual ~ScreenObject() {}

		int x() { return m_x; }
		int y() { return m_y; }
		int width() { return m_width; }
		int height() { return m_height; }

		void setX(int x) { m_x = x; }
		void setY(int y) { m_y = y; }
		void setWidth(int width) { m_width = width; }
		void setHeight(int height) { m_height = height; }

		void resize(int w, int h) { m_width = w; m_height = h; }

		virtual void paint(QPainter &p) = 0;

		virtual std::map<int, Pin> &getPins() = 0;

	signals:
		void onUpdated();

	protected:
		int m_x;
		int m_y;
		int m_width;
		int m_height;

};
