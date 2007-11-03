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
#include <kdebug.h>

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

	if(currentIoSched == KSysGuard::Process::None) {
		currentIoSched = (int)KSysGuard::Process::BestEffort;
		// CurrentIoSched == 0 means that the priority is set automatically.
		// Using the formula given by the linux kernel Documentation/block/ioprio
		currentIoPrio = (currentCpuPrio+20)/5;
	}

	QWidget *widget = new QWidget(this);
	setMainWidget(widget);
	ui = new Ui_ReniceDlgUi();
	ui->setupUi(widget);
	ui->listWidget->insertItems(0, processes);
	ui->spinCPU->setValue(currentCpuPrio);
	ui->spinIO->setValue(currentIoPrio);
	ui->sliderIO->setValue(currentIoPrio);
	if(currentCpuSched == (int)KSysGuard::Process::Other || currentCpuSched == (int)KSysGuard::Process::Batch || currentCpuSched <= 0)
		ui->sliderCPU->setValue(-currentCpuPrio);
	else
		ui->sliderCPU->setValue(currentCpuPrio);

	cpuScheduler = new QButtonGroup(this);
	cpuScheduler->addButton(ui->radioNormal, (int)KSysGuard::Process::Other);
	cpuScheduler->addButton(ui->radioBatch, (int)KSysGuard::Process::Batch);
	cpuScheduler->addButton(ui->radioFIFO, (int)KSysGuard::Process::Fifo);
	cpuScheduler->addButton(ui->radioRR, (int)KSysGuard::Process::RoundRobin);
	if(currentCpuSched >= 0) { //negative means none of these
		QAbstractButton *sched = cpuScheduler->button(currentCpuSched);
		if(sched)
			sched->setChecked(true); //Check the current scheduler
	}
	cpuScheduler->setExclusive(true);

	ioScheduler = new QButtonGroup(this);
	ioScheduler->addButton(ui->radioIdle, (int)KSysGuard::Process::Idle);
	ioScheduler->addButton(ui->radioRealTime, (int)KSysGuard::Process::RealTime);
	ioScheduler->addButton(ui->radioBestEffort, (int)KSysGuard::Process::BestEffort);
	if(currentIoSched >= 0) { //negative means none of these
		QAbstractButton *iosched = ioScheduler->button(currentIoSched);
		if(iosched)
		       	iosched->setChecked(true); //Check the current io scheduler
	}

	ioScheduler->setExclusive(true);

	ui->imgCPU->setPixmap( KIcon("cpu").pixmap(128, 128) );
	ui->imgIO->setPixmap( KIcon("drive-harddisk").pixmap(128, 128) );

       	newPriority = 40;

	connect(cpuScheduler, SIGNAL(clicked(int)), this, SLOT(updateUi()));
	connect(ioScheduler, SIGNAL(clicked(int)), this, SLOT(updateUi()));
	connect(ui->sliderCPU, SIGNAL(sliderMoved(int)), this, SLOT(cpuSliderChanged(int)));
	connect(ui->spinCPU, SIGNAL(valueChanged(int)), this, SLOT(cpuSpinChanged(int)));
	
	updateUi();
}

void ReniceDlg::cpuSliderChanged(int value) {
	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch)
		ui->spinCPU->setValue(-value);
	else
		ui->spinCPU->setValue(value);
	if( ioScheduler->checkedId() == -1 || ioScheduler->checkedId() == (int)KSysGuard::Process::BestEffort) {
		ui->spinIO->setValue((value+20)/5);
		ui->sliderIO->setValue((value+20)/5);
	}
}

void ReniceDlg::cpuSpinChanged(int value) {
	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch)
		ui->sliderCPU->setValue(-value);
	else
		ui->sliderCPU->setValue(value);
	if( ioScheduler->checkedId() == -1 || ioScheduler->checkedId() == (int)KSysGuard::Process::BestEffort) {
		ui->spinIO->setValue((value+20)/5);
		ui->sliderIO->setValue((value+20)/5);
	}
}
void ReniceDlg::updateUi() {
	bool cpuPrioEnabled = ( cpuScheduler->checkedId() != -1);
	bool ioPrioEnabled = ( ioScheduler->checkedId() != -1 && ioScheduler->checkedId() != (int)KSysGuard::Process::Idle);

	ui->sliderCPU->setEnabled(cpuPrioEnabled);
	ui->lblCpuLow->setEnabled(cpuPrioEnabled);
	ui->lblCpuHigh->setEnabled(cpuPrioEnabled);
	ui->spinCPU->setEnabled(cpuPrioEnabled);

	ui->sliderIO->setEnabled(ioPrioEnabled);
	ui->lblIOLow->setEnabled(ioPrioEnabled);
	ui->lblIOHigh->setEnabled(ioPrioEnabled);
	ui->spinIO->setEnabled(ioPrioEnabled);
	
	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch)
		ui->sliderCPU->setValue(- ui->spinCPU->value());
	else
		ui->sliderCPU->setValue(ui->spinCPU->value());


	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch) {
		//The slider is setting the priority, so goes from -20 to 19
		ui->spinCPU->setMinimum(-20);
		ui->spinCPU->setMaximum(19);
		ui->sliderCPU->setMinimum(-19);  //We want the left hand side to be low priority, so spinCPU == -sliderCPU
		ui->sliderCPU->setMaximum(20);
	} else {
		ui->spinCPU->setMinimum(1);
		ui->spinCPU->setMaximum(99);
		ui->sliderCPU->setMinimum(1);  //We want the left hand side to be low priority, so spinCPU == -sliderCPU
		ui->sliderCPU->setMaximum(99);
	}

}

void ReniceDlg::slotOk()
{
  newPriority = ui->spinCPU->value();
}

