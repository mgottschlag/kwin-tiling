/*
  Copyright (c) 2002 Laurent Montel <lmontel@mandrakesoft.com>
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
#include <unistd.h>
#include <kdebug.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <qradiobutton.h>
#include "fileshare.h"
#include <qfile.h>
#include <qdir.h>
#include <kstandarddirs.h>

typedef KGenericFactory<KFileShareConfig, QWidget > ShareFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_fileshare, ShareFactory("kcmfileshare") );


KFileShareConfig::KFileShareConfig(QWidget *parent, const char *name, const QStringList &):
    KCModule(ShareFactory::instance(), parent, name)
{
  QBoxLayout *layout = new QVBoxLayout(this,
				       KDialog::marginHint(),
				       KDialog::spacingHint());
  QVButtonGroup *box = new QVButtonGroup( i18n("File Sharing"), this );
  box->layout()->setSpacing( KDialog::spacingHint() );
  connect( box, SIGNAL( clicked( int )), this, SLOT(configChanged()));
  layout->addWidget(box);
  noSharing=new QRadioButton( i18n("Do &not share"), box );
  sharing=new QRadioButton( i18n("&Share"),  box);
  info = new QLabel( this );
  layout->addWidget(info);
  layout->addStretch();

   QString path = QString::fromLatin1("/usr/sbin");
   QString smbExec = KStandardDirs::findExe( QString::fromLatin1("smbd"), path );
   QString nfsExec = KStandardDirs::findExe( QString::fromLatin1("rpc.nfsd"), path );

  if ( nfsExec.isEmpty() && smbExec.isEmpty())
  {
      info->setText(i18n("SMB and NFS servers not installed!"));
      info->show();
      noSharing->setEnabled( false );
      sharing->setEnabled( false );
  }
  else
  {
      info->hide();
      if(getuid() == 0)
          load();
  }

  if(getuid() == 0)
  {
      setButtons(Help|Apply);
  }
  else
  {
      setButtons(Help);
      noSharing->setEnabled( false );
      sharing->setEnabled( false );
  }
}

void KFileShareConfig::load()
{
    QFile file( "/etc/security/fileshare.conf");
    if ( !file.open( IO_ReadWrite ) )
    {
        noSharing->setChecked( false );
        sharing->setChecked( true );
    }
    else
    {
        QString str(QCString(file.readAll()));
        if ( str=="RESTRICT=yes")
        {
            sharing->setChecked( false );
            noSharing->setChecked( true );
        }
        else if( str=="RESTRICT=no")
        {
            sharing->setChecked( true );
            noSharing->setChecked( false );
        }
        else
        {
            sharing->setChecked( false );
            noSharing->setChecked( true );
        }
    }

}

void KFileShareConfig::save()
{
    QDir dir("/etc/security");
    if ( !dir.exists())
        dir.mkdir("/etc/security");

    QCString str;
    //write file
    if ( noSharing->isChecked())
    {
        str="RESTRICT=yes";
    }
    else
    {
        str="RESTRICT=no";
    }
    QFile file("/etc/security/fileshare.conf");
    if ( file.open(IO_WriteOnly))
        file.writeBlock( str.data(), str.size()-1);
    file.close();
}

void KFileShareConfig::defaults()
{
    noSharing->setChecked( true );
    sharing->setChecked( false );
}

QString KFileShareConfig::quickHelp() const
{
    return i18n("<h1>File Sharing</h1><p>This module can be used "
    		    "to enable file sharing over the network using "
				"the \"Network File System\" (NFS) or SMB in Konqueror. "
				"The latter enables you to share your files with Windows(TM) "
				"computers on your network.</p>");
}

#include "fileshare.moc"
