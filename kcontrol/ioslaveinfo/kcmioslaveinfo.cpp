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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kcmioslaveinfo.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qfile.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kdebug.h>
#include <qtextcodec.h>
#include <kgenericfactory.h>

typedef KGenericFactory<KCMIOSlaveInfo, QWidget> SlaveFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_ioslaveinfo, SlaveFactory("kcmioslaveinfo") )

KCMIOSlaveInfo::KCMIOSlaveInfo(QWidget *parent, const char *name, const QStringList &)
               :KCModule(SlaveFactory::instance(), parent,name),m_ioslavesLb(0),m_tfj(0)
{
   QVBoxLayout *layout=new QVBoxLayout(this, 0, KDialog::spacingHint());

   QLabel* label=new QLabel(i18n("Available IO slaves:"),this);
   QHBox *hbox=new QHBox(this);
   m_ioslavesLb=new KListBox(hbox);
   m_ioslavesLb->setMinimumSize(fontMetrics().width("blahfaselwhatever----"),10);
   connect( m_ioslavesLb, SIGNAL( selectionChanged( QListBoxItem * ) ), SLOT( showInfo( QListBoxItem * ) ) );
   //TODO make something useful after 2.1 is released
   m_info=new KTextBrowser(hbox);
   hbox->setSpacing(KDialog::spacingHint());

   layout->addWidget(label);
   layout->addWidget(hbox);
   hbox->setStretchFactor(m_ioslavesLb,1);
   hbox->setStretchFactor(m_info,5);

   QStringList protocols=KProtocolInfo::protocols();
   for (QStringList::Iterator it=protocols.begin(); it!=protocols.end(); it++)
   {
      QString proto = *it;
      m_ioslavesLb->insertItem( SmallIcon( KProtocolInfo::icon( proto )), 
                                proto );
   };
   m_ioslavesLb->sort();
   m_ioslavesLb->setSelected(0, true);

   setButtons(buttons ());
   load();
}


KCMIOSlaveInfo::~KCMIOSlaveInfo() {
}

const KAboutData* KCMIOSlaveInfo::aboutData() const
{
    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmioslaveinfo"),
		I18N_NOOP("KDE Panel System Information Control Module"),
		0, 0, KAboutData::License_GPL,
    		I18N_NOOP("(c) 2001 - 2002 Alexander Neundorf"));

    about->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
    about->addAuthor("George Staikos", 0, "staikos@kde.org");

    return about;
}


void KCMIOSlaveInfo::load() {
   emit changed(false);
}

void KCMIOSlaveInfo::defaults() {
   emit changed(true);
}

void KCMIOSlaveInfo::save() {
   emit changed(true);
}

int KCMIOSlaveInfo::buttons () {
   return KCModule::Help;
}

void KCMIOSlaveInfo::configChanged() {
   emit changed(true);
}

void KCMIOSlaveInfo::childChanged(bool really) {
   emit changed(really);
}

QString KCMIOSlaveInfo::quickHelp() const
{
   return i18n("<h1>IO slaves</h1> Gives you an overview of the installed ioslaves.");
}

void KCMIOSlaveInfo::slaveHelp( KIO::Job *, const QByteArray &data)
{
    if ( data.size() == 0 ) { // EOF
        int index = helpData.find( "<meta http-equiv=\"Content-Type\"" );
        index = helpData.find( "charset=", index ) + 8;
        QString charset = helpData.mid( index, helpData.find( '\"', index ) - index );
        QString text = QTextCodec::codecForName(charset.latin1())->toUnicode( helpData );
        index = text.find( "<div class=\"article\">" );
        text = text.mid( index );
        index = text.find( "<div id=\"bottom-nav\"" );
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
       m_tfj = KIO::get( KURL( QString("help:/kioslave/%1.html").arg( protocol ) ), true, false );
       connect( m_tfj, SIGNAL( data( KIO::Job *, const QByteArray &) ), SLOT( slaveHelp( KIO::Job *, const QByteArray &) ) );
       connect( m_tfj, SIGNAL( result( KIO::Job * ) ), SLOT( slotResult( KIO::Job * ) ) );
       return;
   }
   m_info->setText(i18n("Some info about protocol %1:/ ...").arg(protocol));
}

void KCMIOSlaveInfo::showInfo(QListBoxItem *item)
{
   if (item==0)
      return;
   showInfo( item->text() );
}

#include "kcmioslaveinfo.moc"

