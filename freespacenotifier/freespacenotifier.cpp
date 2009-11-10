/* This file is part of the KDE Project
   Copyright (c) 2006 Lukas Tinkl <ltinkl@suse.cz>
   Copyright (c) 2008 Lubos Lunak <l.lunak@suse.cz>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "freespacenotifier.h"

#include <sys/vfs.h>
#include <unistd.h>

#include <Qt/qdir.h>
#include <Qt/qfile.h>
#include <Qt/qlabel.h>
#include <Qt/qspinbox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <krun.h>

#include "ui_freespacewidget.h"


FreeSpaceNotifier::FreeSpaceNotifier( QObject* parent )
    : QObject( parent )
    , lastAvailTimer( NULL )
    , dialog( NULL )
    , lastAvail( -1 )
{
    connect( &timer, SIGNAL( timeout() ), SLOT( checkFreeDiskSpace() ) );
    KConfig cfg( "lowspacesuse" );
    KConfigGroup group( &cfg, "General" );
    limit = group.readEntry( "WarnMinimumFreeSpace", 200 ); // MiB
    if( limit != 0 )
        timer.start( 1000 * 60 /* 1 minute */ );
}

FreeSpaceNotifier::~FreeSpaceNotifier()
{
    delete dialog;
}

void FreeSpaceNotifier::checkFreeDiskSpace()
{
    if ( dialog )
        return;
    struct statfs sfs;
    if ( statfs( QFile::encodeName( QDir::homeDirPath() ), &sfs ) == 0 )
    {
        long avail = ( getuid() ? sfs.f_bavail : sfs.f_bfree );

        if (avail < 0 || sfs.f_blocks <= 0)
            return; // we better do not say anything about it
        
        int availpct = int( 100 * avail / sfs.f_blocks );
        avail = ((long long)avail) * sfs.f_bsize / ( 1024 * 1024 ); // to MiB
        bool warn = false;
        if( avail < limit ) // avail disk space dropped under a limit
        {
            if( lastAvail < 0 ) // always warn the first time
            {
                lastAvail = avail;
                warn = true;
            }
            else if( avail > lastAvail ) // the user freed some space
                lastAvail = avail;       // so warn if it goes low again
            else if( avail < lastAvail * 0.5 ) // available dropped to a half of previous one, warn again
            {
                warn = true;
                lastAvail = avail;
            }
            // do not change lastAvail otherwise, to handle free space slowly going down
        }
        if ( warn )
        {
            dialog = new KDialog;
            dialog->setCaption( i18n( "Low Disk Space" ));
            QWidget* mainwidget = new QWidget(dialog);
            dialog->setMainWidget(mainwidget);
            dialog->setButtons( KDialog::Yes | KDialog::No | KDialog::Cancel );
            dialog->setDefaultButton( KDialog::Yes );
            dialog->setEscapeButton( KDialog::No );
            dialog->setButtonText( KDialog::Yes, i18n( "Open File Manager" ));
            dialog->setButtonText( KDialog::No, i18n( "Do Nothing" ));
            dialog->setButtonText( KDialog::Cancel, i18n( "Disable Warning" ));
            widget = new Ui_FreeSpaceWidget();
            widget->setupUi( mainwidget );

            QString text = i18n( "You are running low on disk space on your home partition (currently %2%, %1 MiB free).",
                avail, availpct );
            widget->warningLabel->setText( text );
            widget->spinbox->setMinValue( 0 );
            widget->spinbox->setMaxValue( 100000 );
            widget->spinbox->setValue( limit );
            connect( dialog, SIGNAL( yesClicked() ), SLOT( slotYes() ) );
            connect( dialog, SIGNAL( noClicked() ), SLOT( slotNo() ) );
            connect( dialog, SIGNAL( cancelClicked() ), SLOT( slotCancel() ) );
            dialog->show();
        }
    }
}

void FreeSpaceNotifier::slotYes()
{
    ( void ) new KRun( KUrl::fromPathOrUrl( QDir::homeDirPath() ), dialog );
    cleanupDialog( widget->spinbox->value());
}

void FreeSpaceNotifier::slotNo()
{
    cleanupDialog( widget->spinbox->value());
}

void FreeSpaceNotifier::slotCancel()
{
    cleanupDialog( 0 ); // set limit to zero
}

void FreeSpaceNotifier::cleanupDialog( long newLimit )
{
    dialog->deleteLater();
    dialog = NULL;
    if( limit != newLimit )
    {
        KConfig cfg( "lowspacesuse" );
        KConfigGroup group( &cfg, "General" );
        limit = newLimit;
        group.writeEntry( "WarnMinimumFreeSpace", int( limit ));
        if( limit == 0 )
            timer.stop();
    }
    if( limit != 0 )
    { // warn again if constanly below limit for too long
        if( lastAvailTimer == NULL )
        { 
            lastAvailTimer = new QTimer( this );
            connect( lastAvailTimer, SIGNAL( timeout()), SLOT( resetLastAvailable()));
        }
        lastAvailTimer->start( 1000 * 60 * 60 /* 1 hour*/ ); 
    }
}

void FreeSpaceNotifier::resetLastAvailable()
{
    lastAvail = -1;
    lastAvailTimer->deleteLater();
    lastAvailTimer = NULL;
}

