    /*

    Dialog class to handle input focus -- see headerfile
    $Id$

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */
 

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <klocale.h>

#include "kfdialog.h"

#include <X11/Xlib.h>

FDialog::FDialog( QWidget *parent, const char *name, bool modal )
   : inherited( parent, name, modal, WStyle_NoBorder )
{
    winFrame = new QFrame( this );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->addWidget( winFrame );
}

int
FDialog::exec()
{
    inherited::show();
    XSetInputFocus( qt_xdisplay(), winId(), RevertToParent, CurrentTime );
    inherited::exec();
    if (parentWidget())
	parentWidget()->setActiveWindow();
    return result();
}

KFMsgBox::KFMsgBox( QWidget *parent, QMessageBox::Icon type, const QString &text )
   : inherited( parent )
{
    QLabel *label1 = new QLabel( winFrame );
    label1->setPixmap( QMessageBox::standardIcon( type ) );
    QLabel *label2 = new QLabel( text, winFrame );
    QPushButton *button = new QPushButton( i18n("&OK"), winFrame );
    button->setDefault( true );
    button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    connect( button, SIGNAL( clicked() ), SLOT( accept() ) );

    QGridLayout *grid = new QGridLayout( winFrame, 2, 2, 10 );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    grid->addMultiCellWidget( button, 1,1, 0,1, Qt::AlignCenter );
}

void
KFMsgBox::box( QWidget *parent, QMessageBox::Icon type, const QString &text )
{
    KFMsgBox dlg( parent, type, text.stripWhiteSpace() );
    dlg.exec();
}
