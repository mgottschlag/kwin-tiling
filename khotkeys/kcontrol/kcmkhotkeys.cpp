/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 kcmkhotkeys.cpp  -
 
 $Id$

****************************************************************************/

#define __kcmkhotkeys_CPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksimpleconfig.h>
#include <kaccel.h>
#include <kdialogbase.h>
#include <dcopclient.h>
#include <kkeydialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include "khotkeysglobal.h"

#include "kcmkhotkeys.h"

void khotkeys_init()
    {
    KGlobal::locale()->insertCatalogue("khotkeys");
    }

QString khotkeys_get_desktop_file_shortcut( const QString& file_P )
    {
    KHotData_dict data;
    KSimpleConfig cfg( "khotkeysrc", true );
    data.read_config( cfg );
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        if( it.current()->run == file_P )
            return it.current()->shortcut;
    return "";
    }

bool khotkeys_desktop_file_moved( const QString& new_P, const QString& old_P )
    {
    KHotData_dict data;
        {
        KSimpleConfig cfg( "khotkeysrc", true );
        data.read_config( cfg );
        }
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        if( it.current()->run == new_P )
            return false;
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        if( it.current()->run == old_P )
            {
            it.current()->run = new_P;
            write_conf( data );
            return true;
            }
    return false;
    }

void khotkeys_desktop_file_deleted( const QString& file_P )
    {
    KHotData_dict data;
        {
        KSimpleConfig cfg( "khotkeysrc", true );
        data.read_config( cfg );
        }
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        if( it.current()->run == file_P )
            {
            data.remove( it.currentKey());
            write_conf( data );
            return;
            }
    }

QString khotkeys_change_desktop_file_shortcut( const QString& file_P )
    {
    KHotData_dict data;
        {
        KSimpleConfig cfg( "khotkeysrc", true );
        data.read_config( cfg );
        }
    KHotData* pos = NULL;
    QString name;
    bool new_data = false;
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        if( it.current()->run == file_P )
            {
            name = it.currentKey();
            pos = data.take( name );
            break;
            }
    if( pos == NULL )
        {
        int slash = file_P.findRev( '/' );
        name = kapp->name();
        name += " - ";
        if( slash > -1 )
            name += file_P.mid( slash );
        else
            name += file_P;
        pos = new KHotData( "", file_P );
        new_data = true;
        }
    if( edit_shortcut( name, pos, data ))
        {            
        data.insert( name, pos );
        write_conf( data );
        return pos->shortcut;
        }
    else
        {
        delete pos;
        if( !new_data )
            write_conf( data );
        }
    return "";
    }

void write_conf( KHotData_dict& data_P )
    {
        {
        KSimpleConfig cfg( "khotkeysrc", false );
        data_P.write_config( cfg );
        }
    DCOPClient* client = kapp->dcopClient();
    QByteArray data;
    client->send( "KHotKeys", "KHotKeys", "reread_configuration()", data );
    }
    
bool edit_shortcut( const QString& action_name_P, KHotData* item_P,
    KHotData_dict& data_P )
    {
    desktop_shortcut_dialog* dlg =
        new desktop_shortcut_dialog( action_name_P, item_P, data_P );
    bool ret = dlg->dlg_exec();
    delete dlg;
    return ret;
    }
    
desktop_shortcut_dialog::desktop_shortcut_dialog( const QString& action_name_P,
    KHotData* item_P, KHotData_dict& data_P )
    : KDialogBase( NULL, NULL, true, i18n( "Select shortcut" ), Ok | Cancel ),
        data( data_P ), item( item_P ), action_name( action_name_P )
    {
    KKeyEntry entry;
    entry.aCurrentKeyCode = entry.aConfigKeyCode
        = KAccel::stringToKey( item_P->shortcut );
    entry.bConfigurable = true;
    entry.descr = action_name_P;
    entry.bEnabled = true;
    map.insert( action_name_P, entry );
    QWidget* page = new QWidget( this );
    setMainWidget( page );
    QLabel* label = new QLabel( i18n( "Desktop file to run" ), page );
    QLineEdit* line = new QLineEdit( page );
    line->setText( item_P->run );
    line->setMinimumWidth( 500 ); // CHECKME
    line->setEnabled( false );
    keychooser = new KKeyChooser( &map, page, true );
    connect( keychooser, SIGNAL( keyChange()), this, SLOT( key_changed()));
    QBoxLayout* main_layout = new QVBoxLayout( page, KDialog::marginHint(),
        KDialog::spacingHint());
    main_layout->addWidget( label, 1 );
    main_layout->addWidget( line, 1 );
    main_layout->addWidget( keychooser, 10 );
    }
    
bool desktop_shortcut_dialog::dlg_exec()
    {
    if( exec() == KDialogBase::Accepted )
        item->shortcut
            = KAccel::keyToString( map[ action_name ].aConfigKeyCode );
    return !item->shortcut.isEmpty();
    }

void desktop_shortcut_dialog::key_changed()
    {
    int shortcut = map[ action_name ].aConfigKeyCode;
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        {
        if( it.current() == item )
            continue;
        if( KAccel::stringToKey( it.current()->shortcut ) == shortcut )
            {
            QString str =
                i18n( "The %1 key combination has already "
                     "been allocated\n"
                     "to the %2 action.\n"
                     "\n"
                     "Please choose a unique key combination."
                    ).arg( it.current()->shortcut ).arg( it.currentKey());
            KMessageBox::sorry( this, str, i18n( "Key conflict" ));
            map[ action_name ].aConfigKeyCode = 0;
            keychooser->listSync();
            return;
            }
        }
    }

#include "kcmkhotkeys.moc"
