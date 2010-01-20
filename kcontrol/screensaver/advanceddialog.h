/*
* advanceddialog.h
* Copyright 1997       Matthias Hoelzer
* Copyright 1996,1999,2002    Martin R. Jones
* Copyright 2004       Chris Howells
* Copyright 2007-2008  Benjamin Meyer <ben@meyerhome.net>
* Copyright 2007-2008  Hamish Rodda <rodda@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ADVANCEDDIALOG_H
#define ADVANCEDDIALOG_H

#include <QWidget>
#include <QWhatsThis>
#include <QCheckBox>
#include <QSlider>

#include <KDialog>
#include <KConfig>

#include "ui_advanceddialogimpl.h"

class AdvancedDialogImpl : public QWidget, public Ui::AdvancedDialogImpl
{
public:
  AdvancedDialogImpl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class AdvancedDialog : public AdvancedDialogImpl
{
public:
	AdvancedDialog(QWidget *parent = 0);
	~AdvancedDialog();
	void setMode(QComboBox *box, int i);
	int mode(QComboBox *box);
};

/* =================================================================================================== */

class KScreenSaverAdvancedDialog : public KDialog
{
    Q_OBJECT
public:
    KScreenSaverAdvancedDialog(QWidget *parent);
      
public Q_SLOTS:
    virtual void accept();
         
protected Q_SLOTS:
    void slotPriorityChanged(int val);
    void slotChangeBottomRightCorner(int);
    void slotChangeBottomLeftCorner(int);
    void slotChangeTopRightCorner(int);
    void slotChangeTopLeftCorner(int);
                        
private:
    void readSettings();
                     
    QCheckBox *m_topLeftCorner;
    QCheckBox *m_bottomLeftCorner;
    QCheckBox *m_topRightCorner;
    QCheckBox *m_bottomRightCorner;
    QSlider   *mPrioritySlider;
                                          
    bool mChanged;
    int  mPriority;
    AdvancedDialog *dialog;

};


#endif

