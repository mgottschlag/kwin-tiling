    /*

    Shutdown dialog
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>


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
 

#include <strings.h>
#include "kdmshutdown.h"
#include "kdmconfig.h" // for shutdown-modes

#include "dm.h"
#include "greet.h"

#include <qfile.h>
#include <qcombobox.h>

#include <kapp.h>
#include <klocale.h>

#include "liloinfo.moc"

static inline void
set_min( QWidget* w)
{
     w->adjustSize();
     w->setMinimumSize( w->size());
}
static inline void
set_fixed( QWidget* w)
{
     w->adjustSize();
     w->setFixedSize( w->size());
}

extern KDMConfig *kdmcfg;
extern struct display *d;

KDMShutdown::KDMShutdown( int mode, QWidget* _parent, const char* _name,
			  const QString &_shutdown, 
			  const QString &_restart,
#ifndef BSD
			  const QString &_console,
#endif
			  bool _lilo,
			  const QString &_lilocmd, const QString &_lilomap)

    : FDialog( _parent, _name, true)
{
    shutdown = _shutdown;
    restart  = _restart;
#ifndef BSD
    console = _console;
#endif
    int h = 10, w = 0;
    lilo = _lilo;
    liloCmd = _lilocmd;
    liloMap = _lilomap;
    QFrame* winFrame = new QFrame( this);
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised);
    QBoxLayout* box = new QBoxLayout( winFrame, QBoxLayout::TopToBottom, 
				      10, 10);
    QString shutdownmsg =  i18n( "Shutdown or reboot?");
    if( mode == KDMConfig::SdRootOnly) {
	shutdownmsg += '\n';
	shutdownmsg += i18n( "(Enter Root Password)");
    }
     
    label = new QLabel( shutdownmsg, winFrame);
    set_fixed( label);
    h += label->height() + 10;
    w = label->width();

    box->addWidget( label, 0, AlignCenter);

    QFrame* sepFrame = new QFrame( winFrame);
    sepFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken);
    h += sepFrame->height(); 
    box->addWidget( sepFrame);

    btGroup = new QButtonGroup( /* this */);
     
    QRadioButton *rb;
    rb = new QRadioButton( winFrame /*btGroup*/);
    rb->setText( i18n("&Shutdown"));
    set_min( rb);
    rb->setFocusPolicy( StrongFocus);
    // Default action
    rb->setChecked( true);
    rb->setFocus();
    cur_action = shutdown;

    h += rb->height() + 10;
    w = QMAX( rb->width(), w);

    box->addWidget( rb);
    btGroup->insert( rb);

    QHBoxLayout *hbox = new QHBoxLayout(box);

    restart_rb = new QRadioButton( winFrame /*btGroup*/);
    restart_rb->setText( i18n("&Reboot"));
    set_min( restart_rb);
    restart_rb->setFocusPolicy( StrongFocus);
    h += restart_rb->height() + 10;
    w = QMAX( restart_rb->width(), w);

    hbox->addWidget(restart_rb);

    if ( _lilo ) {
	QComboBox *targets = new QComboBox(winFrame);
	hbox->addWidget(targets);

	// fill combo box with contents of lilo config
	LiloInfo info(_lilocmd, _lilomap);

	QStringList list;
	if (info.getBootOptions(&list) == 0) {
	    targets->insertStringList(list);
            liloTarget = info.getDefaultBootOptionIndex();
	    targets->setCurrentItem(liloTarget);
	    connect(targets,SIGNAL(activated(int)),this,SLOT(target_changed(int)));
	}
    }

    btGroup->insert( restart_rb);

#ifndef BSD
    if (!kdmcfg->_consoleMode.isEmpty()) {
	rb = new QRadioButton(winFrame);
	rb->setText(i18n("Console &Mode"));
	set_min(rb);
	rb->setFocusPolicy(StrongFocus);
	h += rb->height() + 10;
	w = QMAX(rb->width(),w);
	box->addWidget(rb);
	btGroup->insert(rb);
    }
#endif

    // Passwd line edit
    if( mode == KDMConfig::SdRootOnly) {
	pswdEdit = new KPasswordEdit( winFrame, "edit", kdmcfg->_echoMode);
	pswdEdit->setFocusPolicy( StrongFocus);
	pswdEdit->setFocus();
	h+= pswdEdit->height() + 10;
	box->addWidget( pswdEdit);
    } else
	pswdEdit = 0;

    QBoxLayout* box3 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
    box->addLayout( box3);

    okButton = new QPushButton( i18n("&OK"), winFrame);
    set_min( okButton);
    okButton->setDefault( true);
    okButton->setFocusPolicy( StrongFocus);
    cancelButton = new QPushButton( i18n("&Cancel"), winFrame);
    set_min( cancelButton);
    cancelButton->setFocusPolicy( StrongFocus);
    h += cancelButton->height() + 10;
    w = QMAX( (okButton->width() + 10 + cancelButton->width()), w);

    box3->addWidget( okButton);
    box3->addWidget( cancelButton);
    // Connections
    connect( okButton, SIGNAL(clicked()), SLOT(bye_bye()));
    connect( cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect( btGroup, SIGNAL(clicked(int)), SLOT(rb_clicked(int)));

    resize( 20 + w, h);
    winFrame->setGeometry( 0, 0, width(), height());
}

void
KDMShutdown::rb_clicked( int id)
{
    switch( id) {
    case 0:
	cur_action = shutdown;
	break;
    case 1:
	cur_action = restart;
	break;
#ifndef BSD
    case 2:
	cur_action = console;
	break;
#endif
    }
}

void
KDMShutdown::target_changed(int id)
{
    cur_action = restart;
    restart_rb->setChecked(TRUE);
    liloTarget = id;
}

void
KDMShutdown::bye_bye()
{
     // usernames and passwords are stored in the same format as files
    if( !pswdEdit || VerifyRoot( pswdEdit->password() ) ) {
	QApplication::flushX();
	if( fork() == 0) {

	    // if lilo, set the reboot option
	    if (lilo && restart_rb->isChecked())
	    {
		LiloInfo info(liloCmd, liloMap);

		info.setNextBootOption(liloTarget);
	    }

	    sleep(1);
	    system( QFile::encodeName( cur_action ).data() );
	    QApplication::exit( 0);	// init will inherit us
	} else {
	    SessionExit (::d, UNMANAGE_DISPLAY, FALSE);
	}
    } else {
	pswdEdit->erase();
	pswdEdit->setFocus();
	// should show some message ...
    }
}

#include "kdmshutdown.moc"

#ifdef TEST_KDM_SHUTDOWN

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app( argc, argv);
    app.setFont( QFont( "helvetica", 18));
    KDMShutdown sd( 0, 0, "Hej", "echo shutdown", "echo restart", "echo lilo", "");
    app.setMainWidget( &sd);
    return sd.exec();
}

#endif /* TEST_KDM */

/*  
 * Local variables:  
 * mode: c++  
 * c-file-style: "k&r"  
 * End:  
*/
