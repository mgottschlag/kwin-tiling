/*

    $Id$

    Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                  1998 Bernd Wuebben <wuebben@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/


#ifndef __SYSSOUND_H__
#define __SYSSOUND_H__

#include "kcmodule.h"

#include <qstringlist.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qtabwidget.h>

//#include <mediatool.h>
//#include <kaudio.h>
#include <knuminput.h>

class KListBox;
class QLabel;
class QPushButton;

class KSoundWidget : public KCModule{

	Q_OBJECT

public:

	KSoundWidget(QWidget *parent, const char *name);
	virtual ~KSoundWidget();

        void load();
        void save();
	void defaults();
	
	bool eventFilter(QObject *o, QEvent *e);

protected:
	void soundlistDragEnterEvent(QDragEnterEvent *e);
	void soundlistDropEvent(QDropEvent *e);

private slots:
	void eventSelected(int index);
        void soundSelected(int index);
	void playCurrentSound();
	void changed();

        void ringBell();

private:

    void addMainTab();
    void addBellTab();
    void initLists();
    void updateStatus();

    int getBellVolume();
    int getBellPitch();
    int getBellDuration();

    void setBellVolume(int);
    void setBellPitch(int);
    void setBellDuration(int);
	
    int m_selectedEvent;
    QStringList m_soundSettings;
    QStringList m_allSounds;
    QTabWidget *m_tabs;
    QCheckBox *m_soundsEnabled;
    KListBox *m_eventList;
    KListBox *m_soundList;
    QLabel *m_eventLabel;
    QLabel *m_soundLabel;
    QLabel *m_statusText;
    QPushButton *m_testButton;

    KIntNumInput *m_volume;
    KIntNumInput *m_pitch;
    KIntNumInput *m_duration;
    
    QString m_none;
    QString m_i18nNone;
};

#endif
