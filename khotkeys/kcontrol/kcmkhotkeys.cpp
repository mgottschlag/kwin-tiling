/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _KCMKHOTKEYS_CPP_

#include <config-khotkeys.h> // HAVE_ARTS

#include "kcmkhotkeys.h"
#include "khotkeysiface.h"

#include <unistd.h>
#include <stdlib.h>

#include <QLayout>
#include <QSplitter>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kcmodule.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <ktoolinvocation.h>
#include <klibloader.h>

#include <input.h>
#include <triggers.h>
#include <action_data.h>

#include <QtDBus/QtDBus>

#include "tab_widget.h"
#include "actions_listview_widget.h"
#include "main_buttons_widget.h"
#include "voicerecorder.h"

K_PLUGIN_FACTORY(KHotKeysFactory,
        registerPlugin<KHotKeys::Module>();
        )
K_EXPORT_PLUGIN(KHotKeysFactory("khotkeys"))

extern "C"
{
    KDE_EXPORT void kcminit_khotkeys()
    {
    KConfig _cfg( "khotkeysrc" );
    KConfigGroup cfg(&_cfg, "Main" );
    if( !cfg.readEntry( "Autostart", false))
        return;
    // Non-xinerama multhead support in KDE is just a hack
    // involving forking apps per-screen. Don't bother with
    // kded modules in such case.
    QByteArray multiHead = getenv("KDE_MULTIHEAD");
    if (multiHead.toLower() == "true")
        KToolInvocation::kdeinitExec( "khotkeys" );
    else
        {
        QDBusInterface kded("org.kde.kded", "/kded", "org.kde.kded");
        QDBusReply<bool> reply = kded.call("loadModule",QString( "khotkeys" ) );
        if( !reply.isValid())
            {
            kWarning( 1217 ) << "Loading of khotkeys module failed." ;
            KToolInvocation::kdeinitExec( "khotkeys" );
            }
        }
    }
}

namespace KHotKeys
{

Module::Module( QWidget* parent_P, const QVariantList & )
    : KCModule( KHotKeysFactory::componentData(), parent_P ), _actions_root( NULL ), _current_action_data( NULL ),
        listview_is_changed( false ), deleting_action( false )
    {
    setButtons( Apply );
    module = this;
    init_global_data( false, this ); // don't grab keys
    init_arts();
    QVBoxLayout* vbox = new QVBoxLayout( this );
    vbox->setSpacing( KDialog::spacingHint() );
    vbox->setMargin( 0 );
    QSplitter* splt = new QSplitter( this );
    actions_listview_widget = new Actions_listview_widget( splt );
    actions_listview_widget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
    tab_widget = new Tab_widget( splt );
    tab_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    vbox->addWidget( splt );
    splt->setStretchFactor( 0, 0 );
    splt->setStretchFactor( 1, 1 );
    buttons_widget = new Main_buttons_widget( this );
    vbox->addWidget( buttons_widget );
    connect( actions_listview_widget, SIGNAL( current_action_changed()),
        SLOT( listview_current_action_changed()));
    connect( buttons_widget, SIGNAL( new_action_pressed()), SLOT( new_action()));
    connect( buttons_widget, SIGNAL( new_action_group_pressed()), SLOT( new_action_group()));
    connect( buttons_widget, SIGNAL( delete_action_pressed()), SLOT( delete_action()));
    connect( buttons_widget, SIGNAL( global_settings_pressed()), SLOT( global_settings()));
//    listview_current_action_changed(); // init

    KAboutData* about = new KAboutData("kcmkhotkeys", 0, ki18n("KHotKeys"), KHOTKEYS_VERSION,
        KLocalizedString(), KAboutData::License_GPL, ki18n("(c) 1999-2005 Lubos Lunak"));
    about->addAuthor(ki18n("Lubos Lunak"), ki18n("Maintainer"), "l.lunak@kde.org");
    setAboutData( about );
    load();
}

Module::~Module()
    {
    _current_action_data = NULL;
    tab_widget->load_current_action(); // clear tab_widget
    delete _actions_root;
    module = NULL;
    }

void Module::load()
    {
    actions_listview_widget->clear();
    delete _actions_root;
    settings.actions = NULL;
    _current_action_data = NULL;
    settings.read_settings( true );
    _actions_root = settings.actions;
    kDebug( 1217 ) << "actions_root:" << _actions_root;
    actions_listview_widget->build_up();
    tab_widget->load_current_action();
    emit KCModule::changed( false ); // HACK otherwise the module would be changed from the very beginning
    }

void Module::save()
    {
    tab_widget->save_current_action_changes();
    settings.actions = _actions_root;
    kDebug(1217) << "Storing actions" << _actions_root;
    settings.write_settings();
    QDBusConnection bus = QDBusConnection::sessionBus();
    if( daemon_disabled())
        {
        if( bus.interface()->isServiceRegistered( "org.kde.khotkeys" ))
            {
            // wait for it to finish
            org::kde::khotkeys* iface = new org::kde::khotkeys("org.kde.khotkeys", "/KHotKeys", bus, this);
            iface->quit();
            sleep( 1 );
            }
        kDebug( 1217 ) << "disabling khotkeys daemon";
        }
    else
        {
        if( !bus.interface()->isServiceRegistered( "org.kde.khotkeys" ))
            {
            kDebug( 1217 ) << "launching new khotkeys daemon";
            KToolInvocation::kdeinitExec( "khotkeys" );
            }
        else
            {
            org::kde::khotkeys iface("org.kde.khotkeys", "/KHotKeys", bus);
            iface.reread_configuration();
            kDebug( 1217 ) << "telling khotkeys daemon to reread configuration";
            }
        }
    emit KCModule::changed( false );
    }


QString Module::quickHelp() const
    {
    // return i18n( "" ); // TODO CHECKME
    return QString(); // TODO CHECKME
    }

void Module::action_name_changed( const QString& name_P )
    {
    current_action_data()->set_name( name_P );
    actions_listview_widget->action_name_changed( name_P );
    }

void Module::listview_current_action_changed()
    {
    // CHECKME tohle je trosku hack, aby se pri save zmenenych hodnot ve stare vybrane polozce
    // zmenila data v te stare polozce a ne nove aktivni
    listview_is_changed = true;
    set_new_current_action( !deleting_action );
    listview_is_changed = false;
    }

void Module::set_new_current_action( bool save_old_P )
    {
    if( save_old_P )
        tab_widget->save_current_action_changes();
    _current_action_data = actions_listview_widget->current_action_data();
    kDebug( 1217 ) << "set_new_current_action : " << _current_action_data;
    tab_widget->load_current_action();
    buttons_widget->enable_delete( current_action_data() != NULL );
    }

// CHECKME volano jen z Tab_widget pro nastaveni zmenenych dat ( novy Action_data_base )
void Module::set_current_action_data( Action_data_base* data_P )
    {
    delete _current_action_data;
    _current_action_data = data_P;
    actions_listview_widget->set_action_data( data_P, listview_is_changed );
//    tab_widget->load_current_action(); CHECKME asi neni treba
    }

#if 0

}
#include <iostream>
#include <iomanip>
#include <KPluginFactory>
#include <KPluginLoader>
namespace KHotKeys {

void check_tree( Action_data_group* b, int lev_P = 0 )
    {
    using namespace std;
    cerr << setw( lev_P ) << "" << b << ":Group:" << b->name().toLatin1() << ":" << b->parent() << endl;
    for( Action_data_group::Iterator it = b->first_child();
         it;
         ++it )
        if( Action_data_group* g = dynamic_cast< Action_data_group* >( *it ))
            check_tree( g, lev_P + 1 );
        else
            cerr << setw( lev_P + 1 ) << "" << (*it) << ":Action:" << (*it)->name().toLatin1() << ":" << (*it)->parent() << endl;
    }

#endif

void Module::new_action()
    {
    tab_widget->save_current_action_changes();
//    check_tree( actions_root());
    Action_data_group* parent = current_action_data() != NULL
        ? dynamic_cast< Action_data_group* >( current_action_data()) : NULL;
    if( parent == NULL )
        {
        if( current_action_data() != NULL )
            parent = current_action_data()->parent();
        else
            parent = module->actions_root();
        }
    Action_data_base* item = new Generic_action_data( parent, i18n( "New Action" ), "",
        new Trigger_list( "" ), new Condition_list( "", NULL ), new Action_list( "" ), true );
    actions_listview_widget->new_action( item );
//    check_tree( actions_root());
    set_new_current_action( false );
    }

// CHECKME spojit tyhle dve do jedne
void Module::new_action_group()
    {
    tab_widget->save_current_action_changes();
//    check_tree( actions_root());
    Action_data_group* parent = current_action_data() != NULL
        ? dynamic_cast< Action_data_group* >( current_action_data()) : NULL;
    if( parent == NULL )
        {
        if( current_action_data() != NULL )
            parent = current_action_data()->parent();
        else
            parent = module->actions_root();
        }
    Action_data_base* item = new Action_data_group( parent, i18n( "New Action Group" ), "",
        new Condition_list( "", NULL ), Action_data_group::SYSTEM_NONE, true );
    actions_listview_widget->new_action( item );
//    check_tree( actions_root());
    set_new_current_action( false );
    }

void Module::delete_action()
    {
    delete _current_action_data;
    _current_action_data = NULL;
    deleting_action = true; // CHECKME zase tak trosku hack, jinak by se snazilo provest save
    actions_listview_widget->delete_action(); // prave mazane polozky
    deleting_action = false;
    set_new_current_action( false );
    }

void Module::global_settings()
    {
    actions_listview_widget->set_current_action( NULL );
    set_new_current_action( true );
    }

void Module::set_gestures_exclude( Windowdef_list* windows )
    {
    delete settings.gestures_exclude;
    settings.gestures_exclude = windows;
    }

void Module::import()
    {
    QString file = KFileDialog::getOpenFileName( QString(), "*.khotkeys", window(),
        i18n( "Select File with Actions to Be Imported" ));
    if( file.isEmpty())
        return;
    KConfig cfg(  file, KConfig::SimpleConfig);
    if( !settings.import( cfg, true ))
        {
        KMessageBox::error( window(),
            i18n( "Import of the specified file failed. Most probably the file is not a valid "
                "file with actions." ));
        return;
        }
    actions_listview_widget->clear();
    actions_listview_widget->build_up();
    tab_widget->load_current_action();
    emit KCModule::changed( true );
    }

void Module::changed()
    {
    emit KCModule::changed( true );
    }

void Module::init_arts()
    {
#warning Port to Phonon
#ifdef HAVE_ARTS
    if( haveArts())
        {
        KLibrary* arts = KLibLoader::self()->library( QLatin1String("khotkeys_arts") );
        if( arts == NULL )
            kDebug( 1217 ) << "Couldn't load khotkeys_arts:" << KLibLoader::self()->lastErrorMessage();
        if( arts != NULL && VoiceRecorder::init( arts ))
            ; // ok
        else
            disableArts();
        }
#endif
    }

Module* module; // CHECKME

} // namespace KHotKeys

#include "kcmkhotkeys.moc"
