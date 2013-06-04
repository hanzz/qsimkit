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

#include "QSimKit.h"

#include "ProjectConfiguration.h"

#include "MCU/MCU.h"
#include "Peripherals/PeripheralManager.h"

#include "DockWidgets/Disassembler/Disassembler.h"
// #include "DockWidgets/Registers/Registers.h"
// #include "DockWidgets/Stack/Stack.h"
// #include "DockWidgets/MemoryTracker/MemoryTracker.h"
#include "DockWidgets/Peripherals/Peripherals.h"

#include "Breakpoints/BreakpointManager.h"

#include "Tracking/TrackedPins.h"

#include <QWidget>
#include <QTime>
#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QIcon>
#include <QIODevice>
#include <QDebug>
#include <QDomDocument>

QSimKit::QSimKit(QWidget *parent) : QMainWindow(parent),
m_dig(0), m_sim(0), m_logicalSteps(0), m_instPerCycle(2500) {
	setupUi(this);

	m_peripherals = new PeripheralManager();
	m_peripherals->loadPeripherals();

	screen->setPeripheralManager(m_peripherals);

	m_breakpointManager = new BreakpointManager();

	connect(actionLoad_A43, SIGNAL(triggered()), this, SLOT(loadA43()) );
	connect(actionLoad_ELF, SIGNAL(triggered()), this, SLOT(loadELF()) );
	connect(actionNew_project, SIGNAL(triggered()), this, SLOT(newProject()) );
	connect(actionSave_project, SIGNAL(triggered()), this, SLOT(saveProject()) );
	connect(actionLoad_project, SIGNAL(triggered()), this, SLOT(loadProject()) );
	connect(actionProject_options, SIGNAL(triggered()), this, SLOT(projectOptions()) );
	connect(actionTracked_pins, SIGNAL(triggered()), this, SLOT(showTrackedPins()) );

	QAction *action = toolbar->addAction(QIcon("./icons/22x22/actions/media-playback-start.png"), tr("Start &simulation"));
	connect(action, SIGNAL(triggered()), this, SLOT(startSimulation()));

	action = toolbar->addAction(QIcon("./icons/22x22/actions/media-playback-pause.png"), tr("P&ause simulation"));
	action->setCheckable(true);
	action->setEnabled(false);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(pauseSimulation(bool)));
	m_pauseAction = action;

	action = toolbar->addAction(QIcon("./icons/22x22/actions/media-playback-stop.png"), tr("Sto&p simulation"));
	connect(action, SIGNAL(triggered()), this, SLOT(stopSimulation()));

	action = toolbar->addAction(QIcon("./icons/22x22/actions/media-skip-forward.png"), tr("Single step"));
	connect(action, SIGNAL(triggered()), this, SLOT(singleStep()));

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(simulationStep()));

	m_disassembler = new Disassembler(this);
	addDockWidget(m_disassembler, Qt::RightDockWidgetArea);
// 	addDockWidget(new Registers(this), Qt::LeftDockWidgetArea);
// 	addDockWidget(new MemoryTracker(this), Qt::LeftDockWidgetArea);
// 	addDockWidget(new Stack(this), Qt::LeftDockWidgetArea);
	m_peripheralsWidget = new Peripherals(this);
	addDockWidget(m_peripheralsWidget, Qt::LeftDockWidgetArea);

	connect(screen, SIGNAL(onPeripheralAdded(QObject *)), m_peripheralsWidget, SLOT(addPeripheral(QObject *)));
	connect(screen, SIGNAL(onPeripheralRemoved(QObject *)), m_peripheralsWidget, SLOT(removePeripheral(QObject *)));

	m_trackedPins = new TrackedPins(this, this);
	QMainWindow::addDockWidget(Qt::BottomDockWidgetArea, m_trackedPins);

	setDockWidgetsEnabled(false);
}

Screen *QSimKit::getScreen() {
	return screen;
}

void QSimKit::showTrackedPins() {
	m_trackedPins->show();
}

void QSimKit::addDockWidget(DockWidget *widget, Qt::DockWidgetArea area) {
	QMainWindow::addDockWidget(area, widget);
	m_dockWidgets.append(widget);
}

void QSimKit::refreshDockWidgets() {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->refresh();
	}

	statusbar->showMessage(QString::number(m_sim->nextEventTime()));
}

void QSimKit::setDockWidgetsEnabled(bool enabled) {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->setEnabled(enabled);
	}

	toolbar->setEnabled(enabled);
}

void QSimKit::setDockWidgetsMCU(MCU *mcu) {
	for (int i = 0; i < m_dockWidgets.size(); ++i) {
		m_dockWidgets[i]->setMCU(mcu);
	}

	setDockWidgetsEnabled(true);
}

void QSimKit::singleStep() {
	if (!m_dig) {
		resetSimulation();
	}

	double t = m_sim->nextEventTime();

	do {
		m_sim->execNextEvent();
	}
	while (t == m_sim->nextEventTime());
	refreshDockWidgets();

	onSimulationStep(m_sim->nextEventTime());
}

void QSimKit::simulationStep() {
	QTime perf;
	perf.start();
	for (int i = 0; i < m_instPerCycle; ++i) {
		m_sim->execNextEvent();
		if (m_breakpointManager->shouldBreak()) {
			m_pauseAction->setChecked(true);
			pauseSimulation(true);
			return;
		}
	}
	m_logicalSteps++;
	if (m_logicalSteps == 2) {
		m_logicalSteps = 0;
		statusbar->showMessage(QString("Simulation Time: ") + QString::number(m_sim->nextEventTime()) + ", " + QString::number(m_instPerCycle * 20) + " simulation events per second");
		onSimulationStep(m_sim->nextEventTime());
	}

	
	// Keep 60% CPU usage. This method is called every 50ms, so we should spend
	// 30ms here. If we are spending less or more, change the instPerCycle
	// value.
	if (perf.elapsed() < 28 || perf.elapsed() > 32) {
		m_instPerCycle = m_instPerCycle * (30.0 / perf.elapsed());
	}
}

void QSimKit::resetSimulation() {
	m_timer->stop();
	m_pauseAction->setChecked(false);

	delete m_dig;
	delete m_sim;

	m_dig = new adevs::Digraph<double>();
	screen->prepareSimulation(m_dig);
	m_sim = new adevs::Simulator<SimulationEvent>(m_dig);
	screen->setSimulator(m_sim);
	
}

void QSimKit::startSimulation() {
	if (m_pauseAction->isChecked()) {
		m_pauseAction->setChecked(false);
		onSimulationStarted(true);
	}
	else {
		resetSimulation();
		onSimulationStarted(false);
	}
	m_pauseAction->setEnabled(true);
	m_timer->start(50);
}

void QSimKit::stopSimulation() {
	m_timer->stop();
	m_pauseAction->setEnabled(false);
	onSimulationStopped();
}

void QSimKit::pauseSimulation(bool checked) {
	if (checked) {
		m_timer->stop();
		refreshDockWidgets();
		onSimulationPaused();
	}
	else {
		m_timer->start(50);
	}
}

void QSimKit::newProject() {
	ProjectConfiguration dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		m_filename = "";
		screen->clear();
		screen->setMCU(dialog.getMCU());
		setDockWidgetsMCU(screen->getMCU());

		m_breakpointManager->setMCU(screen->getMCU());
	}
}

void QSimKit::saveProject() {
	if (m_filename.isEmpty()) {
		m_filename = QFileDialog::getSaveFileName(this);
		if (m_filename.isEmpty()) {
			return;
		}
	}

	QFile file(m_filename);
	if (!file.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
		return;

	QTextStream stream(&file);
	stream << "<qsimkit_project>\n";
	screen->save(stream);
	stream << "</qsimkit_project>\n";
}

bool QSimKit::loadProject(const QString &file) {
    int errorLine, errorColumn;
    QString errorMsg;

	QFile modelFile(file);
	QDomDocument document;
	if (!document.setContent(&modelFile, &errorMsg, &errorLine, &errorColumn))
	{
			QString error("Syntax error line %1, column %2:\n%3");
			error = error
					.arg(errorLine)
					.arg(errorColumn)
					.arg(errorMsg);
			qDebug() << error;
			return false;
	}

	screen->load(document);
	m_filename = file;
	setDockWidgetsMCU(screen->getMCU());
	m_breakpointManager->setMCU(screen->getMCU());

	return true;
}

void QSimKit::loadProject() {
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty()) {
		return;
	}

	loadProject(filename);
}

bool QSimKit::loadA43File(const QString &f) {
	if (!screen->getMCU()) {
		newProject();
		if (!screen->getMCU()) {
			return false;
		}
	}

	QFile file(f);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	bool ret = screen->getMCU()->loadA43(file.readAll().data());
	m_disassembler->reloadCode();
	return ret;
}

void QSimKit::loadA43() {
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty()) {
		return;
	}

	loadA43File(filename);
}

bool QSimKit::loadELFFile(const QString &f) {
	if (!screen->getMCU()) {
		newProject();
		if (!screen->getMCU()) {
			return false;
		}
	}

	QFile file(f);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	QByteArray elf = file.readAll();
	QString a43 = m_disassembler->ELFToA43(elf);

	screen->getMCU()->loadELF(elf);

	qDebug() << a43;
	bool ret = screen->getMCU()->loadA43(a43.toAscii().data());
	m_disassembler->reloadCode();
	return ret;
}

void QSimKit::loadELF() {
	QString filename = QFileDialog::getOpenFileName(this);
	if (filename.isEmpty()) {
		return;
	}

	if (!loadELFFile(filename)) {
		qDebug() << "Error while loading ELF file";
	}
}

void QSimKit::projectOptions() {
	ProjectConfiguration dialog(this, screen->getMCU());
	if (dialog.exec() == QDialog::Accepted) {

	}
}
