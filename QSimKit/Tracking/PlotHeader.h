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

#include <QWidget>
#include <QString>
#include <QList>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

class PlotHeader : public QWidget
{
	Q_OBJECT

	public:
		PlotHeader(QWidget *parent = 0);
		~PlotHeader();

		void addPin(const QString &label);

		void clear();

	public slots:
		void showTable(bool checked = false);
		void handleRedIndexChanged(int);
		void handleGreenIndexChanged(int);

	signals:
		void onPinChanged(int color, int pin);
		void onShowTable();

	private:
		QHBoxLayout *m_layout;
		QLabel *m_label;
		QComboBox *m_redPin;
		QComboBox *m_greenPin;
		int m_index;

};

