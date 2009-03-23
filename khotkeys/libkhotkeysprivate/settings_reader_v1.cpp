/**
 * Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 **/

#include "settings_reader_v1.h"

#include "action_data/action_data_group.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/command_url_shortcut_action_data.h"
#include "conditions/conditions_list.h"

#include <KDE/KConfigGroup>


namespace KHotKeys {

SettingsReaderV1::SettingsReaderV1(Settings *settings)
    :   _settings(settings)
    {}


SettingsReaderV1::~SettingsReaderV1()
    {}


void SettingsReaderV1::read(
        const KConfigBase &config,
        ActionDataGroup *parent)
    {
    KConfigGroup mainGroup( &config, "Main" );
    int sections = mainGroup.readEntry( "Num_Sections", 0 );
    ActionDataGroup* menuentries = NULL;
    Q_FOREACH (ActionDataBase *child, parent->children())
        {
        ActionDataGroup* tmp = dynamic_cast< ActionDataGroup* >(child);
        if( tmp == NULL )
            continue;
        if( tmp->system_group() == ActionDataGroup::SYSTEM_MENUENTRIES )
            {
            menuentries = tmp;
            break;
            }
        }
    for( int sect = 1;
         sect <= sections;
         ++sect )
        {
        QString group = QString( "Section%1" ).arg( sect );
        if( !config.hasGroup( group ))
            continue;
        KConfigGroup sectionConfig( &config, group );
        QString name = sectionConfig.readEntry( "Name" );
        if( name.isNull() )
            continue;
        QString shortcut = sectionConfig.readEntry( "Shortcut" );
        if( shortcut.isNull() )
            continue;
        QString run = sectionConfig.readEntry( "Run" );
        if( run.isNull() )
            continue;
        bool menuentry = sectionConfig.readEntry( "MenuEntry", false);
        // CHECKME tohle pridavani az pak je trosku HACK
        if( menuentry )
            {
            if( menuentries == NULL )
                {
                menuentries = new ActionDataGroup( parent,
                    i18n( MENU_EDITOR_ENTRIES_GROUP_NAME ),
                    i18n( "These entries were created using Menu Editor." ), NULL,
                    ActionDataGroup::SYSTEM_MENUENTRIES, true );
                menuentries->set_conditions( new Condition_list( "", menuentries ));
                }
            ( void ) new MenuEntryShortcutActionData( menuentries, name, "",
                KShortcut( shortcut ), run );
            }
        else
            {
            ( void ) new CommandUrlShortcutActionData( parent, name, "",
                KShortcut( shortcut ), run );
            }
        }
    }


} // namespace KHotKeys
