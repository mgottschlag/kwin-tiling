/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 kcmkhotkeys.h  -
 
 $Id$

****************************************************************************/

#ifndef __kcmkhotkeys_H
#define __kcmkhotkeys_H

extern "C"
    {
void khotkeys_init( void );
QString khotkeys_get_desktop_file_shortcut( const QString& file_P );
QString khotkeys_change_desktop_file_shortcut( const QString& file_P );
bool khotkeys_desktop_file_moved( const QString& new_P, const QString& old_P );
void khotkeys_desktop_file_deleted( const QString& file_P );    
    } // extern "C"

static void write_conf( KHotData_dict& data_P );
static bool edit_shortcut( const QString& action_name_P, KHotData* data_P,
    KHotData_dict& data_P );

class desktop_shortcut_dialog
    : public KDialogBase
    {
    Q_OBJECT
    public:
        desktop_shortcut_dialog( const QString& action_name_P,
            KHotData* item_P, KHotData_dict& data_P );
        bool dlg_exec();
    protected slots:
        void key_changed();
    protected:
        KKeyEntryMap map;
        KHotData_dict& data;
        KHotData* item;
        QString action_name;
        KKeyChooser* keychooser;
    };

#endif
