    /*

    Shutdown dialog. Class KDMShutdown

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2002 Oswald Buddenhagen <ossi@kde.org>


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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */


#ifndef KDMSHUTDOWN_H
#define KDMSHUTDOWN_H

#include "kfdialog.h"

#include <qradiobutton.h>

class LiloInfo;
class QLabel;
class KPushButton;
class QButtonGroup;
class KPasswordEdit;
class QComboBox;
class QTimer;

class KDMShutdown : public FDialog {
    Q_OBJECT
    typedef FDialog inherited;

public:
    KDMShutdown( QWidget *_parent = 0 );
#if defined(__linux__) && (defined(__i386__) || defined(__amd64__) )
    ~KDMShutdown();
#endif

private slots:
    void bye_bye();
    void target_changed();
    void when_changed( int );
    void timerDone();

private:
    QLabel		*label;
    QButtonGroup	*howGroup, *whenGroup;
    KPushButton		*okButton;
    KPushButton		*cancelButton;
    KPasswordEdit	*pswdEdit;
    QRadioButton	*restart_rb, *force_rb, *try_rb;
    QTimer		*timer;
    bool		needRoot;
#if defined(__linux__) && ( defined(__i386__)  || defined(__amd64__) )
    LiloInfo		*liloInfo;
    QComboBox		*targets;
    int			defaultLiloTarget, oldLiloTarget;
#endif

};

class KDMRadioButton : public QRadioButton
{
    Q_OBJECT
    typedef QRadioButton inherited;

public:
    KDMRadioButton( const QString &label, QWidget *parent );

private:
    virtual void mouseDoubleClickEvent( QMouseEvent * );

signals:
    void doubleClicked();

};

#endif /* KDMSHUTDOWN_H */
