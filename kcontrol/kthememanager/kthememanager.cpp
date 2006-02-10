// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/*  Copyright (C) 2003 Lukas Tinkl <lukas@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QBoxLayout>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kemailsettings.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <k3urldrag.h>

#include "kthememanager.h"
#include "knewthemedlg.h"
#include "config.h"

kthememanager::kthememanager( KInstance *inst, QWidget *parent )
    : KCModule( inst, parent ), m_theme( 0 ), m_origTheme( 0 )
{

    KAboutData *about = new KAboutData("kthememanager", I18N_NOOP("KDE Theme Manager"),
                                       "0.4", I18N_NOOP("This control module handles installing, removing and "
                                                        "creating visual KDE themes."),
                                       KAboutData::License_GPL, "(c) 2003, 2004 Lukáš Tinkl", 0,
                                       "http://developer.kde.org/~lukas/kthememanager");
    setAboutData( about );

    setQuickHelp( i18n("This control module handles installing, removing and "
                "creating visual KDE themes."));

    setButtons( KCModule::Default|KCModule::Apply|KCModule::Help );

    setAcceptDrops( true );
    init();

    QBoxLayout *top = new QVBoxLayout(this, 0, KDialog::spacingHint());

    dlg = new KThemeDlg(this);
    top->addWidget( dlg );

    dlg->lvThemes->setColumnWidthMode( 0, Q3ListView::Maximum );

    connect( ( QObject * )dlg->btnInstall, SIGNAL( clicked() ),
             this, SLOT( slotInstallTheme() ) );

    connect( ( QObject * )dlg->btnRemove, SIGNAL( clicked() ),
             this, SLOT( slotRemoveTheme() ) );

    connect( ( QObject * )dlg->btnCreate, SIGNAL( clicked() ),
             this, SLOT( slotCreateTheme() ) );

    connect( ( QObject * )dlg->lvThemes, SIGNAL( clicked( Q3ListViewItem * ) ),
             this, SLOT( slotThemeChanged( Q3ListViewItem * ) ) );

    connect( this, SIGNAL( filesDropped( const KUrl::List& ) ),
             this, SLOT( updateButton() ) );

    connect( ( QObject * )dlg->lvThemes, SIGNAL( clicked( Q3ListViewItem * ) ),
             this, SLOT( updateButton() ) );

    m_origTheme = new KTheme( this, true ); // stores the defaults to get back to
    m_origTheme->setName( ORIGINAL_THEME );
    m_origTheme->createYourself();

    load();
    queryLNFModules();
    updateButton();
}

kthememanager::~kthememanager()
{
    delete m_theme;
    delete m_origTheme;
}

void kthememanager::init()
{
    KGlobal::dirs()->addResourceType( "themes", KStandardDirs::kde_default("data") +
                                      "kthememanager/themes/" );
}

void kthememanager::updateButton()
{
    Q3ListViewItem * cur = dlg->lvThemes->currentItem();
    dlg->btnRemove->setEnabled( cur != 0 );
}

void kthememanager::load()
{
    listThemes();

    // Load the current theme name
    KConfig conf("kcmthememanagerrc", false, false);
    conf.setGroup( "General" );
    QString themeName = conf.readEntry( "CurrentTheme" );
    Q3ListViewItem * cur =  dlg->lvThemes->findItem( themeName, 0 );
    if ( cur )
    {
        dlg->lvThemes->setSelected( cur, true );
        dlg->lvThemes->ensureItemVisible( cur );
        slotThemeChanged( cur );
    }
}

void kthememanager::defaults()
{
    if ( m_origTheme )
        m_origTheme->apply();
}

void kthememanager::save()
{
    Q3ListViewItem * cur = dlg->lvThemes->currentItem();

    if ( cur )
    {
        QString themeName = cur->text( 0 );

        m_theme = new KTheme( this, KGlobal::dirs()->findResource( "themes", themeName + "/" + themeName + ".xml") );
        m_theme->apply();

        // Save the current theme name
        KConfig conf("kcmthememanagerrc", false, false);
        conf.setGroup( "General" );
        conf.writeEntry( "CurrentTheme", themeName );
        conf.sync();

        delete m_theme;
        m_theme = 0;

    }
}

void kthememanager::listThemes()
{
    dlg->lvThemes->clear();
    dlg->lbPreview->setPixmap( QPixmap() );

    QStringList themes = KGlobal::dirs()->findAllResources( "themes", "*.xml", true /*recursive*/ );

    QStringList::const_iterator it;

    for ( it = themes.begin(); it != themes.end(); ++it )
    {
        KTheme theme( this, ( *it ) );
        QString name = theme.name();
        if ( name != ORIGINAL_THEME ) // skip the "original" theme
            ( void ) new Q3ListViewItem( dlg->lvThemes, name, theme.comment() );
    }

    kDebug() << "Available themes: " << themes << endl;
}

float kthememanager::getThemeVersion( const QString & themeName )
{
    QStringList themes = KGlobal::dirs()->findAllResources( "themes", "*.xml", true /*recursive*/ );

    QStringList::const_iterator it;

    for ( it = themes.begin(); it != themes.end(); ++it )
    {
        KTheme theme( 0L, ( *it ) );
        QString name = theme.name();
        bool ok = false;
        float version = theme.version().toFloat( &ok );
        if ( name == themeName && ok )
            return version;
    }

    return -1;
}

void kthememanager::slotInstallTheme()
{
    addNewTheme( KFileDialog::getOpenURL( ":themes", "*.kth|" + i18n("Theme Files"), this,
                                          i18n( "Select Theme File" ) ) );
}

void kthememanager::addNewTheme( const KUrl & url )
{
    if ( url.isValid() )
    {
        QString themeName = QFileInfo( url.fileName() ).baseName();
        if ( getThemeVersion( themeName ) != -1 ) // theme exists already
        {
            KTheme::remove( themeName  ); // remove first
        }

        m_theme = new KTheme(this);
        if ( m_theme->load( url ) )
        {
            listThemes();
            emit changed( true );
        }

        delete m_theme;
        m_theme = 0;
        updateButton();
    }
}

void kthememanager::slotRemoveTheme()
{
    // get the selected item from the listview
    Q3ListViewItem * cur = dlg->lvThemes->currentItem();
    // ask and remove it
    if ( cur )
    {
        QString themeName = cur->text( 0 );
        if ( KMessageBox::warningContinueCancel( this, "<qt>" + i18n( "Do you really want to remove the theme <b>%1</b>?" ).arg( themeName ),
                                         i18n( "Remove Theme" ),KGuiItem(i18n("&Remove"),"editdelete") )
             == KMessageBox::Continue )
        {
            KTheme::remove( themeName );
            listThemes();
        }
        updateButton();
    }
}

bool kthememanager::themeExist(const QString &_themeName)
{
    return ( dlg->lvThemes->findItem( _themeName, 0 )!=0 );
}

void kthememanager::slotCreateTheme()
{
    KNewThemeDlg dlg( this );

    KEMailSettings es;
    es.setProfile( es.defaultProfileName() );

    dlg.setName( i18n( "My Theme" ) );
    dlg.setAuthor( es.getSetting( KEMailSettings::RealName ) ) ;
    dlg.setEmail( es.getSetting( KEMailSettings::EmailAddress ) );
    dlg.setVersion( "0.1" );

    if ( dlg.exec() == QDialog::Accepted )
    {

        QString themeName = dlg.getName();
        if ( themeExist(themeName) )
        {
            KMessageBox::information( this, i18n( "Theme %1 already exists." ).arg( themeName ) );
        }
        else
        {
            if ( getThemeVersion( themeName ) != -1 ) // remove the installed theme first
            {
                KTheme::remove( themeName );
            }
            m_theme = new KTheme( this, true );
            m_theme->setName( dlg.getName() );
            m_theme->setAuthor( dlg.getAuthor() );
            m_theme->setEmail( dlg.getEmail() );
            m_theme->setHomepage( dlg.getHomepage() );
            m_theme->setComment( dlg.getComment().replace( "\n", "" ) );
            m_theme->setVersion( dlg.getVersion() );

            m_theme->addPreview();
            QString result = m_theme->createYourself( true );

            if ( !result.isEmpty() )
                KMessageBox::information( this, i18n( "Your theme has been successfully created in %1." ).arg( result ),
                                          i18n( "Theme Created" ), "theme_created_ok" );
            else
                KMessageBox::error( this, i18n( "An error occurred while creating your theme." ),
                                    i18n( "Theme Not Created" ) );
            delete m_theme;
            m_theme = 0;

            listThemes();
        }
    }
}

void kthememanager::slotThemeChanged( Q3ListViewItem * item )
{
    if ( item )
    {
        QString themeName = item->text(0);
        kDebug() << "Activated theme: " << themeName  << endl;

	QString themeDir = KGlobal::dirs()->findResourceDir( "themes", themeName + "/" + themeName + ".xml") + themeName + "/";

        QString pixFile = themeDir + themeName + ".preview.png";

        if ( QFile::exists( pixFile ) )
        {
            updatePreview( pixFile );
        }
        else
        {
            dlg->lbPreview->setPixmap( QPixmap() );
            dlg->lbPreview->setText( i18n( "This theme does not contain a preview." ) );
        }

        KTheme theme( this, themeDir + themeName + ".xml" );
        dlg->lbPreview->setToolTip( "<qt>" + i18n( "Author: %1<br>Email: %2<br>Version: %3<br>Homepage: %4" )
                       .arg( theme.author() ).arg( theme.email() )
                       .arg( theme.version() ).arg( theme.homepage() ) + "</qt>");

        emit changed( true );
    }
}

void kthememanager::dragEnterEvent( QDragEnterEvent * ev )
{
    ev->accept( K3URLDrag::canDecode( ev ) );
}

void kthememanager::dropEvent( QDropEvent * ev )
{
    KUrl::List urls;
    if ( K3URLDrag::decode( ev, urls ) )
    {
        emit filesDropped( urls );
    }
}

void kthememanager::slotFilesDropped( const KUrl::List & urls )
{
    for ( KUrl::List::ConstIterator it = urls.begin(); it != urls.end(); ++it )
        addNewTheme( *it );
}

void kthememanager::queryLNFModules()
{
    /*KServiceGroup::Ptr settings = KServiceGroup::group( "Settings/LookNFeel/" );
    if ( !settings || !settings->isValid() )
        return;

    KServiceGroup::List list = settings->entries();

    // Iterate over all entries in the group
    for( KServiceGroup::List::ConstIterator it = list.begin();
         it != list.end(); it++ )
    {
        KSycocaEntry *p = ( *it );
        if ( p->isType( KST_KService ) )
        {
            KService *s = static_cast<KService *>( p );
            ( void ) new KThemeDetailsItem( dlg->lvDetails, s->name(), s->pixmap( KIcon::Desktop ), s->exec() );
        }
    }

    dlg->lvDetails->sort();*/

    // For now use a static list
    KIconLoader * il = KGlobal::iconLoader();
    dlg->btnBackground->setIconSet( il->loadIconSet( "background", KIcon::Desktop, 32 ) );
    dlg->btnColors->setIconSet( il->loadIconSet( "colorscm", KIcon::Desktop, 32 ) );
    dlg->btnStyle->setIconSet( il->loadIconSet( "style", KIcon::Desktop, 32 ) );
    dlg->btnIcons->setIconSet( il->loadIconSet( "icons", KIcon::Desktop, 32 ) );
    dlg->btnFonts->setIconSet( il->loadIconSet( "fonts", KIcon::Desktop, 32 ) );
    dlg->btnSaver->setIconSet( il->loadIconSet( "kscreensaver", KIcon::Desktop, 32 ) );
}

void kthememanager::updatePreview( const QString & pixFile )
{
     kDebug() << "Preview is in file: " << pixFile << endl;
     QImage preview( pixFile, "PNG" );
     if (preview.width()>dlg->lbPreview->contentsRect().width() ||
         preview.height()>dlg->lbPreview->contentsRect().height() )
         preview = preview.smoothScale( dlg->lbPreview->contentsRect().size(), Qt::KeepAspectRatio );
     QPixmap pix;
     pix.convertFromImage( preview );
     dlg->lbPreview->setPixmap( pix );
}

extern "C"
{
    KDE_EXPORT KCModule *create_kthememanager(QWidget *parent, const char *)
    {
        KInstance *inst = new KInstance( "kthememanager" );
        return new kthememanager( inst, parent );
    }
}

#include "kthememanager.moc"
