/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2007 John Tapsell <tapsell@kde.org>

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

#include <klocale.h>

#include "ReniceDlg.moc"
#include <QListWidget>
#include <QSpinBox>
#include <QButtonGroup>
#include "ui_ReniceDlgUi.h"
#include "processcore/process.h"

ReniceDlg::ReniceDlg(QWidget* parent, int currentCpuPrio, int currentCpuSched, int currentIoPrio, int currentIoSched, const QStringList& processes)
	: KDialog( parent )
{
	setObjectName( "Renice Dialog" );
	setModal( true );
	setCaption( i18n("Renice Process") );
	setButtons( Ok | Cancel );
	showButtonSeparator( true );

	connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );

	QWidget *widget = new QWidget(this);
	setMainWidget(widget);
	reniceDlgUi = new Ui_ReniceDlgUi();
	reniceDlgUi->setupUi(widget);
	reniceDlgUi->listWidget->insertItems(0, processes);
	reniceDlgUi->spinCPU->setValue(currentCpuPrio);
	reniceDlgUi->spinIO->setValue(currentIoPrio);

	cpuScheduler = new QButtonGroup(this);
	cpuScheduler->addButton(reniceDlgUi->radioNormal, (int)KSysGuard::Process::Other);
	cpuScheduler->addButton(reniceDlgUi->radioBatch, (int)KSysGuard::Process::Batch);
	cpuScheduler->addButton(reniceDlgUi->radioFIFO, (int)KSysGuard::Process::Fifo);
	cpuScheduler->addButton(reniceDlgUi->radioRR, (int)KSysGuard::Process::RoundRobin);
	if(currentCpuSched >= 0) { //negative means none of these
		QAbstractButton *sched = cpuScheduler->button(currentCpuSched);
		if(sched) sched->setChecked(true); //Check the current scheduler
	}
	cpuScheduler->setExclusive(true);

	ioScheduler = new QButtonGroup(this);
	ioScheduler->addButton(reniceDlgUi->radioIdle, (int)KSysGuard::Process::Idle);
	ioScheduler->addButton(reniceDlgUi->radioRealTime, (int)KSysGuard::Process::RealTime);
	ioScheduler->addButton(reniceDlgUi->radioBestEffort, (int)KSysGuard::Process::BestEffort);
	if(currentIoSched >= 0) { //negative means none of these
		QAbstractButton *iosched = ioScheduler->button(currentIoSched);
		if(iosched) iosched->setChecked(true); //Check the current io scheduler
	}
	ioScheduler->setExclusive(true);

	reniceDlgUi->imgCPU->setPixmap( KIcon("cpu").pixmap(128, 128) );
	reniceDlgUi->imgIO->setPixmap( KIcon("drive-harddisk").pixmap(128, 128) );


       	newPriority = 40;
}

void ReniceDlg::slotOk()
{
  newPriority = reniceDlgUi->spinCPU->value();
}

