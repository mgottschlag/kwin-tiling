/*
    $Id$

    Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qvbox.h>

#include <dcopclient.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <knotifydialog.h>
#include <kparts/genericfactory.h>
#include <kstandarddirs.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>


#include "knotify.h"

static const int COL_FILENAME = 1;

typedef KGenericFactory<KCMKNotify, QWidget> NotifyFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_knotify, NotifyFactory("kcmnotify") );

using namespace KNotify;

KCMKNotify::KCMKNotify(QWidget *parent, const char *name, const QStringList & )
    : KCModule(NotifyFactory::instance(), parent, name),
      m_playerSettings( 0L )
{
    setButtons( Help | Default | Apply );

    QVBoxLayout *layout = new QVBoxLayout( this );
    // layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    m_appCombo = new KComboBox( false, this, "app combo" );
    
    m_notifyWidget = new KNotifyWidget( this, "knotify widget" );
    connect( m_notifyWidget, SIGNAL( changed( bool )), SIGNAL( changed(bool)));
    
    layout->addWidget( m_appCombo );
    layout->addWidget( m_notifyWidget );
    
    connect( m_appCombo, SIGNAL( activated( const QString& ) ),
             SLOT( slotAppActivated( const QString& )) );

    connect( m_notifyWidget->m_playerButton, SIGNAL( clicked() ), 
             SLOT( slotPlayerSettings()));
    
    load();

};

KCMKNotify::~KCMKNotify()
{
    KConfig config( "knotifyrc", false, false );
    config.setGroup( "Misc" );
    config.writeEntry( "LastConfiguredApp", m_appCombo->currentText() );
    config.sync();
}

Application * KCMKNotify::applicationByDescription( const QString& text )
{
    // not really efficient, but this is not really time-critical
    ApplicationList apps = m_notifyWidget->apps();
    ApplicationListIterator it ( apps );
    for ( ; it.current(); ++it )
    {
        if ( it.current()->text() == text )
            return it.current();
    }
    
    return 0L;
}

void KCMKNotify::slotAppActivated( const QString& text )
{
    Application *app = applicationByDescription( text );
    if ( app )
        m_notifyWidget->setCurrentApplication( app );
}

void KCMKNotify::slotPlayerSettings()
{
    // kcmshell is a modal dialog, and apparently, we can't put a non-modal
    // dialog besides a modal dialog. sigh.
    if ( !m_playerSettings )
        m_playerSettings = new PlayerSettingsDialog( this, true );
    
    m_playerSettings->exec();
}


// FIXME: this doesn't work, it's a Reset, not a Defaults.
void KCMKNotify::defaults()
{
    if (KMessageBox::warningContinueCancel(this,
        i18n("This will cause the notifications for *All Applications* "
             "to be reset to their defaults!"), i18n("Are you sure?!"), i18n("Continue"))
        != KMessageBox::Continue)
        return;

    m_notifyWidget->reload( true );
}

void KCMKNotify::load()
{
    setEnabled( false );
    // setCursor( KCursor::waitCursor() );

    m_appCombo->clear();
    m_notifyWidget->clear();
    
    m_fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/eventsrc", false, true );

    QStringList::ConstIterator it = m_fullpaths.begin();
    for ( ; it != m_fullpaths.end(); ++it) {
        m_notifyWidget->addApplicationEvents( *it );
    }

    ApplicationList apps = m_notifyWidget->apps();
    apps.sort();
    m_notifyWidget->setEnabled( !apps.isEmpty() );
    
    ApplicationListIterator appIt( apps );
    for ( ; appIt.current(); ++appIt ) 
        m_appCombo->insertItem( appIt.current()->text() );
    
    
    KConfig config( "knotifyrc", true, false );
    config.setGroup( "Misc" );
    QString appDesc = config.readEntry( "LastConfiguredApp" );
    if ( !appDesc.isEmpty() )
        m_appCombo->setCurrentItem( appDesc );
    
    slotAppActivated( m_appCombo->currentText() );

    
    // unsetCursor(); // unsetting doesn't work. sigh.
    setEnabled( true );
}

void KCMKNotify::save()
{
    if ( m_playerSettings )
        m_playerSettings->save();
    
    m_notifyWidget->save(); // will dcop knotify about its new config

    emit changed( false );
}

QString KCMKNotify::quickHelp() const
{
    return i18n("<h1>System Notifications</h1>"
                "KDE allows for a great deal of control over how you "
                "will be notified when certain events occur. There are "
                "several choices as to how you are notified:"
                "<ul><li>As the application was originally designed."
                "<li>With a beep or other noise."
                "<li>Via a popup dialog box with additional information."
                "<li>By recording the event in a logfile without "
                "any additional visual or audible alert."
                "</ul>");
}

const KAboutData *KCMKNotify::aboutData() const
{
    static KAboutData* ab = 0;

    if(!ab)
    {
        ab = new KAboutData(
            "kcmnotify", I18N_NOOP("KNotify"), "3.0",
            I18N_NOOP("System Notification Control Panel Module"),
            KAboutData::License_GPL, "(c) 2002 Carsten Pfeiffer", 0, 0 );
        ab->addAuthor( "Carsten Pfeiffer", 0, "pfeiffer@kde.org" );
        ab->addCredit( "Charles Samuels", I18N_NOOP("Original implementation"),
                       "charles@altair.dhs.org" );
    }

    return ab;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

PlayerSettingsDialog::PlayerSettingsDialog( QWidget *parent, bool modal )
    : KDialogBase( parent, "player settings dialog", modal,
                   i18n("Player Settings"), Ok|Apply|Cancel, Ok, true )
{
    QVBox *box = new QVBox( this );
    setMainWidget( box );
    
    QHBox *hbox = new QHBox( box );
    hbox->setSpacing( KDialog::spacingHint() );
    cbExternal = new QCheckBox( i18n("Use e&xternal player: "), hbox );
    reqExternal = new KURLRequester( hbox );
    reqExternal->completionObject()->setMode( KURLCompletion::ExeCompletion );
    connect( cbExternal, SIGNAL( toggled( bool )),
             SLOT( externalToggled( bool )));

    hbox = new QHBox( box );
    hbox->setSpacing( KDialog::spacingHint() );
    QLabel *l = new QLabel( i18n( "&Volume: " ), hbox );
    volumeSlider = new QSlider( hbox );
    volumeSlider->setOrientation( Horizontal );
    volumeSlider->setRange( 0, 100 );
    l->setBuddy( volumeSlider );
    
    load();
}

void PlayerSettingsDialog::load()
{
    KConfig config( "knotifyrc", true, false );
    config.setGroup( "Misc" );
    cbExternal->setChecked( config.readBoolEntry( "Use external player", 
                                                  false ));
    reqExternal->setURL( config.readEntry( "External player" ));
    reqExternal->setEnabled( cbExternal->isChecked() );
    volumeSlider->setValue( config.readNumEntry( "Volume", 100 ) );
    volumeSlider->parentWidget()->setEnabled( !cbExternal->isChecked() );
}

void PlayerSettingsDialog::save()
{
    // see kdelibs/arts/knotify/knotify.cpp
    KConfig config( "knotifyrc", false, false );
    config.setGroup( "Misc" );
    config.writeEntry( "External player", reqExternal->url() );
    config.writeEntry( "Use external player", cbExternal->isChecked() );
    config.writeEntry( "Volume", volumeSlider->value() );
    config.sync();
}

// reimplements KDialogBase::slotApply()
void PlayerSettingsDialog::slotApply()
{
    save();
    kapp->dcopClient()->send("knotify", "", "reconfigure()", ""); 
    
    KDialogBase::slotApply();
}

// reimplements KDialogBase::slotOk()
void PlayerSettingsDialog::slotOk()
{
    slotApply();
    KDialogBase::slotOk();
}

void PlayerSettingsDialog::externalToggled( bool on )
{
    reqExternal->setEnabled( on );
    volumeSlider->parentWidget()->setEnabled( !on );

    if ( on )
        reqExternal->setFocus();
}

#include "knotify.moc"
