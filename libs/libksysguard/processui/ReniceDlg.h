/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2006 John Tapsell <tapsell@kde.org>

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

#ifndef _ReniceDlg_h_
#define _ReniceDlg_h_

#include <kdialog.h>

class Ui_ReniceDlgUi;
class QButtonGroup;

/**
 * This class creates and handles a simple dialog to change the scheduling
 * priority of a process.
 */
class ReniceDlg : public KDialog
{
	Q_OBJECT

public:
	ReniceDlg(QWidget* parent, int currentCpuPrio, int currentCpuSched, int currentIoPrio, int currentIoSched, const QStringList& processes);
	int newCPUPriority;
	int newIOPriority;
	int newCPUSched;
	int newIOSched;


public Q_SLOTS:
	void slotOk();
	void updateUi();
	void cpuSliderChanged(int value);

private:
	Ui_ReniceDlgUi *ui;
	QButtonGroup *cpuScheduler;
	QButtonGroup *ioScheduler;
};

#endif
