/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/


#include "shutdown.h"
#include <qapplication.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpainter.h>
#include <qtimer.h>
#include <X11/Xlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KSMShutdown::KSMShutdown()
    : QDialog( 0, 0, TRUE, WStyle_Customize | WStyle_NoBorder ) //WType_Popup )
{
    QVBoxLayout* vbox = new QVBoxLayout( this );
    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    frame->setLineWidth( style().defaultFrameWidth() );
    vbox->addWidget( frame );
    vbox = new QVBoxLayout( frame, 15, 5 );

    QLabel* label = new QLabel(
         "<center><b><big><big>Shutdown KDE Session?</big></big></b></center>",
	 frame );
    vbox->addWidget( label );
    vbox->addStretch();

    checkbox = new QCheckBox( "&Restore session when loggin in next time", frame );
    vbox->addWidget( checkbox, 0, AlignRight  );
    vbox->addStretch();

    QHBoxLayout* hbox = new QHBoxLayout( vbox );
    hbox->addStretch();
    QPushButton* yes = new QPushButton("&Yes", frame );
    connect( yes, SIGNAL( clicked() ), this, SLOT( accept() ) );
    yes->setDefault( TRUE );
    hbox->addWidget( yes );
    QPushButton* cancel = new QPushButton("&Cancel", frame );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    hbox->addWidget( cancel );


    QTimer::singleShot( 0, this, SLOT( requestFocus() ) );
    checkbox->setFocus();
}

KSMShutdown::~KSMShutdown()
{
}


void KSMShutdown::requestFocus()
{
    XSetInputFocus( qt_xdisplay(), winId(), RevertToParent, CurrentTime );
}

bool KSMShutdown::shutdown( bool& saveSession )
{
    QWidget* w = new QWidget(0, 0, WStyle_Customize | WStyle_NoBorder);
    w->setBackgroundMode( QWidget::NoBackground );
    w->setGeometry( QApplication::desktop()->geometry() );
    w->show();
    QPainter p;
    QBrush b( Dense4Pattern );
    p.begin( w );
    p.fillRect( w->rect(), b);
    p.end();
    KSMShutdown* l = new KSMShutdown;
    l->checkbox->setChecked( saveSession );
    l->show();
    saveSession = l->checkbox->isChecked();
    bool result = l->result();
    delete l;
    delete w;
    return result;
}
