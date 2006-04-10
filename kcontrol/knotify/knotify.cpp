/*
    Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2005,2006 Olivier Goffart <ogoffart at kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <QVBoxLayout>
#include <QFrame>
#include <QHBoxLayout>

#include <dcopclient.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <knotifyconfigwidget.h>
#include <kparts/genericfactory.h>
#include <kstandarddirs.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>


#include "knotify.h"
#include "playersettings.h"

static const int COL_FILENAME = 1;

typedef KGenericFactory<KCMKNotify, QWidget> NotifyFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_knotify, NotifyFactory("kcmnotify") )

		KCMKNotify::KCMKNotify(QWidget *parent, const char */*name*/, const QStringList & )
    : KCModule(NotifyFactory::instance(), parent/*, name*/),
      m_playerSettings( 0L )
{
    setButtons( Help | Default | Apply );

    setQuickHelp( i18n("<h1>System Notifications</h1>"
                "KDE allows for a great deal of control over how you "
                "will be notified when certain events occur. There are "
                "several choices as to how you are notified:"
                "<ul><li>As the application was originally designed."
                "<li>With a beep or other noise."
                "<li>Via a popup dialog box with additional information."
                "<li>By recording the event in a logfile without "
                "any additional visual or audible alert."
                "</ul>"));

	QVBoxLayout *layout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
	QTabWidget *tab = new QTabWidget(this);
	layout->addWidget(tab);

	QWidget * app_tab = new QWidget(tab);
	QVBoxLayout *app_layout = new QVBoxLayout( app_tab );
	
	QLabel *label = new QLabel( i18n( "Event source:" ), app_tab );
	m_appCombo = new KComboBox( false, app_tab );
	m_appCombo->setObjectName( "app combo" );
	QHBoxLayout *hbox = new QHBoxLayout( app_layout );
	hbox->addWidget( label );
	hbox->addWidget( m_appCombo, 10 );

	m_notifyWidget = new KNotifyConfigWidget( app_tab );
	app_layout->addWidget( m_notifyWidget );

//    connect( m_notifyWidget, SIGNAL( changed( bool )), SIGNAL( changed(bool)));
	changed (true);
	
	m_playerSettings = new PlayerSettingsDialog(tab);

/*	general->layout()->setMargin( KDialog::marginHint() );
	hardware->layout()->setMargin( KDialog::marginHint() );*/
	tab->addTab(app_tab, i18n("&Applications"));
	tab->addTab(m_playerSettings, i18n("&Player settings"));

    connect( m_appCombo, SIGNAL( activated( const QString& ) ),
             SLOT( slotAppActivated( const QString& )) );

    KAboutData* ab = new KAboutData(
        "kcmknotify", I18N_NOOP("KNotify"), "4.0",
        I18N_NOOP("System Notification Control Panel Module"),
        KAboutData::License_GPL, "(c) 2002-2006 KDE Team", 0, 0 );
	
	ab->addAuthor( "Olivier Goffart", 0, "ogoffart@kde.org" );
    ab->addAuthor( "Carsten Pfeiffer", 0, "pfeiffer@kde.org" );
    ab->addCredit( "Charles Samuels", I18N_NOOP("Original implementation"),
	       "charles@altair.dhs.org" );
    setAboutData( ab );

    load();
	

}

KCMKNotify::~KCMKNotify()
{
    KConfig config( "knotifyrc", false, false );
    config.setGroup( "Misc" );
    config.writeEntry( "LastConfiguredApp", m_appCombo->currentText() );
    config.sync();
}

void KCMKNotify::slotAppActivated( const QString& text )
{
	m_notifyWidget->save();
	m_notifyWidget->setApplication( text );
	emit changed(true);
}

void KCMKNotify::slotPlayerSettings()
{
}


void KCMKNotify::defaults()
{
//    m_notifyWidget->resetDefaults( true ); // ask user
}

void KCMKNotify::load()
{
    //setEnabled( false );
    // setCursor( KCursor::waitCursor() );

    m_appCombo->clear();
//    m_notifyWidget->clear();

    QStringList fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/*.notifyrc", false, true );

	foreach (const QString &fullPath, fullpaths )
	{
		int slash = fullPath.lastIndexOf( '/' ) - 1;
		int slash2 = fullPath.lastIndexOf( '/', slash );
		QString appname= slash2 < 0 ? QString() :  fullPath.mid( slash2+1 , slash-slash2  );
		if ( !appname.isEmpty() )
			m_appCombo->insertItem( appname );
	}
	/*        
    KConfig config( "knotifyrc", true, false );
    config.setGroup( "Misc" );
    QString appDesc = config.readEntry( "LastConfiguredApp", "KDE System Notifications" );
    
    if this code gets enabled again, make sure to apply r494965
    
    if ( !appDesc.isEmpty() )
        m_appCombo->setCurrentItem( appDesc );

     // sets the applicationEvents for KNotifyWidget
    slotAppActivated( m_appCombo->currentText() );

    // unsetCursor(); // unsetting doesn't work. sigh.
    setEnabled( true );
    emit changed( false );
	*/
	
	m_playerSettings->load();
	
	emit changed(true);
	
}

void KCMKNotify::save()
{
    if ( m_playerSettings )
        m_playerSettings->save();

    m_notifyWidget->save(); // will dcop knotify about its new config
	m_playerSettings->save();

    emit changed( true );
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

PlayerSettingsDialog::PlayerSettingsDialog( QWidget *parent )
	: QWidget(parent)
{

	m_ui = new Ui::PlayerSettingsUI();
	m_ui->setupUi( this );

    load();
    dataChanged = false;

    connect( m_ui->cbExternal, SIGNAL( toggled( bool ) ), this, SLOT( externalToggled( bool ) ) );
    connect( m_ui->grpPlayers, SIGNAL( clicked( int ) ), this, SLOT( slotChanged() ) );
    connect( m_ui->volumeSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( slotChanged() ) );
    connect( m_ui->reqExternal, SIGNAL( textChanged( const QString& ) ), this, SLOT( slotChanged() ) );
}

void PlayerSettingsDialog::load()
{
    KConfig config( "knotifyrc", true, false );
    config.setGroup( "Misc" );
    bool useExternal = config.readEntry( "Use external player", false );
    m_ui->cbExternal->setChecked( useExternal );
    m_ui->reqExternal->setURL( config.readPathEntry( "External player" ) );
    m_ui->volumeSlider->setValue( config.readEntry( "Volume", 100 ) );

    if ( !m_ui->cbExternal->isChecked() )
    {
        config.setGroup( "StartProgress" );
        if ( config.readEntry( "Use Arts", true ) )
        {
            m_ui->cbArts->setChecked( true );
        }
        else
        {
            m_ui->cbNone->setChecked( true );
        }
    }
}

void PlayerSettingsDialog::save()
{
    // see kdelibs/arts/knotify/knotify.cpp
    KConfig config( "knotifyrc", false, false );
    config.setGroup( "Misc" );

    config.writePathEntry( "External player", m_ui->reqExternal->url() );
    config.writeEntry( "Use external player", m_ui->cbExternal->isChecked() );
    config.writeEntry( "Volume", m_ui->volumeSlider->value() );

    config.setGroup( "StartProgress" );

    if ( m_ui->cbNone->isChecked() )
    {
        // user explicitly says "no sound!"
        config.writeEntry( "Use Arts", false );
    }
    else if ( m_ui->cbArts->isChecked() )
    {
        // use explicitly said to use aRts so we turn it back on
        // we don't want to always set this to the value of
        // m_ui->cbArts->isChecked() since we don't want to
        // turn off aRts support just because they also chose
        // an external player
        config.writeEntry( "Use Arts", true );
        config.writeEntry( "Arts Init", true ); // reset it for the next time
    }

    config.sync();
}

// reimplements KDialogBase::slotApply()
void PlayerSettingsDialog::slotApply()
{
/*    save();
    dataChanged = false;
    enableButton(Apply, false);
    kapp->dcopClient()->send("knotify", "", "reconfigure()", QString());*/
}

// reimplements KDialogBase::slotOk()
void PlayerSettingsDialog::slotOk()
{
/*    if( dataChanged )
        slotApply();
    KDialogBase::slotOk();*/
}

void PlayerSettingsDialog::slotChanged()
{
    dataChanged = true;
//    enableButton(Apply, true);
}

void PlayerSettingsDialog::externalToggled( bool on )
{
    if ( on )
        m_ui->reqExternal->setFocus();
    else
        m_ui->reqExternal->clearFocus();
}

PlayerSettingsDialog::~ PlayerSettingsDialog( )
{
	delete m_ui;
}

#include "knotify.moc"
