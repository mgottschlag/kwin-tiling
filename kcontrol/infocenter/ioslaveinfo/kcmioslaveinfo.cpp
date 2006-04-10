/*
 * kcmioslaveinfo.cpp
 *
 * Copyright 2001 Alexander Neundorf <neundorf@kde.org>
 * Copyright 2001 George Staikos  <staikos@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtextcodec.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>

#include "kcmioslaveinfo.h"

typedef KGenericFactory<KCMIOSlaveInfo, QWidget> SlaveFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ioslaveinfo, SlaveFactory("kcmioslaveinfo") )

KCMIOSlaveInfo::KCMIOSlaveInfo(QWidget *parent, const char *name, const QStringList &)
               :KCModule(SlaveFactory::instance(), parent),m_ioslavesLb(0),m_tfj(0)
{
   QVBoxLayout *layout=new QVBoxLayout(this, 0, KDialog::spacingHint());

   setQuickHelp( i18n("<h1>IO slaves</h1> Gives you an overview of the installed ioslaves."));
   setButtons( KCModule::Help );

   QLabel* label=new QLabel(i18n("Available IO slaves:"),this);
   QWidget *hbox=new QWidget(this);
   QHBoxLayout *hboxLayout1 = new QHBoxLayout(hbox);
   hbox->setLayout(hboxLayout1);
   m_ioslavesLb=new KListBox(hbox);
   m_ioslavesLb->setMinimumSize(fontMetrics().width("blahfaselwhatever----"),10);
   hboxLayout1->addWidget( m_ioslavesLb );
   connect( m_ioslavesLb, SIGNAL( selectionChanged( Q3ListBoxItem * ) ), SLOT( showInfo( Q3ListBoxItem * ) ) );
   //TODO make something useful after 2.1 is released
   m_info=new KTextBrowser(hbox);
   hboxLayout1->setSpacing(KDialog::spacingHint());
   hboxLayout1->addWidget( m_info );

   layout->addWidget(label);
   layout->addWidget(hbox);
   hboxLayout1->setStretchFactor(m_ioslavesLb,1);
   hboxLayout1->setStretchFactor(m_info,5);

   QStringList protocols=KProtocolInfo::protocols();
   for (QStringList::Iterator it=protocols.begin(); it!=protocols.end(); ++it)
   {
      QString proto = *it;
      m_ioslavesLb->insertItem( SmallIcon( KProtocolInfo::icon( proto )),
                                proto );
   };
   m_ioslavesLb->sort();
   m_ioslavesLb->setSelected(0, true);

   setButtons(KCModule::Help);

   KAboutData *about =
   new KAboutData(I18N_NOOP("kcmioslaveinfo"),
	I18N_NOOP("KDE Panel System Information Control Module"),
	0, 0, KAboutData::License_GPL,
    	I18N_NOOP("(c) 2001 - 2002 Alexander Neundorf"));

   about->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
   about->addAuthor("George Staikos", 0, "staikos@kde.org");
   setAboutData( about );

}

void KCMIOSlaveInfo::slaveHelp( KIO::Job *, const QByteArray &data)
{
    if ( data.size() == 0 ) { // EOF
        int index = helpData.indexOf( "<meta http-equiv=\"Content-Type\"" );
        index = helpData.indexOf( "charset=", index ) + 8;
        QString charset = helpData.mid( index, helpData.indexOf( '\"', index ) - index );
        QString text = QTextCodec::codecForName(charset.latin1())->toUnicode( helpData );
        index = text.indexOf( "<div class=\"titlepage\">" );
        text = text.mid( index );
        index = text.indexOf( "<table width=\"100%\" class=\"bottom-nav\"" );
        text = text.left( index );
        m_info->setText(text);
        return;
    }
    helpData += data;
}

void KCMIOSlaveInfo::slotResult(KIO::Job *)
{
   m_tfj = 0;
}

void KCMIOSlaveInfo::showInfo(const QString& protocol)
{
   QString file = QString("kioslave/%1.docbook").arg( protocol );
   file = KGlobal::locale()->langLookup( file );
   if (m_tfj)
   {
      m_tfj->kill();
      m_tfj = 0;
   }

   if (!file.isEmpty())
   {
       helpData.truncate( 0 );
       m_tfj = KIO::get( KUrl( QString("help:/kioslave/%1.html").arg( protocol ) ), true, false );
       connect( m_tfj, SIGNAL( data( KIO::Job *, const QByteArray &) ), SLOT( slaveHelp( KIO::Job *, const QByteArray &) ) );
       connect( m_tfj, SIGNAL( result( KIO::Job * ) ), SLOT( slotResult( KIO::Job * ) ) );
       return;
   }
   m_info->setText(i18n("Some info about protocol %1:/ ...", protocol));
}

void KCMIOSlaveInfo::showInfo(Q3ListBoxItem *item)
{
   if (item==0)
      return;
   showInfo( item->text() );
}

#include "kcmioslaveinfo.moc"

