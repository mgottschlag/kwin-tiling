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


#include "settings_writer.h"

#include "action_data/action_data_group.h"
#include "settings.h"
#include "windows_helper/window_selection_list.h"


#include <KDE/KConfig>
#include <KDE/KConfigGroup>

namespace KHotKeys {

const int CurrentFileVersion = 2;


SettingsWriter::SettingsWriter(const Settings *settings)
    :   _settings(settings)
    {
    }


void SettingsWriter::exportTo(const ActionDataBase *element, KConfigBase &config)
    {
    if (!element)
        {
        Q_ASSERT(element);
        return;
        }

    // Clean the file
    QStringList groups = config.groupList();
    Q_FOREACH (QString name, config.groupList())
        {
        config.deleteGroup(name);
        }

    KConfigGroup mainGroup(&config, "Main");
    mainGroup.writeEntry("Version", CurrentFileVersion);

    // The root group contains nothing but the datacount!
    KConfigGroup dataGroup(&config,  "Data");
    dataGroup.writeEntry("DataCount", 1);

    // The group for the element to export
    KConfigGroup data1Group(&config,  "Data_1");
    element->cfg_write(data1Group);

    const ActionDataGroup *group = dynamic_cast<const ActionDataGroup*>(element);
    if (group)
        {
        write_actions(data1Group, group, true);
        }
    }


void SettingsWriter::writeTo(KConfigBase &cfg)
    {
    QStringList groups = cfg.groupList();
    for( QStringList::ConstIterator it = groups.constBegin();
         it != groups.constEnd();
         ++it )
        cfg.deleteGroup( *it );
    KConfigGroup mainGroup( &cfg, "Main" ); // main group
    mainGroup.writeEntry( "Version", 2 ); // now it's version 2 cfg. file
    mainGroup.writeEntry( "AlreadyImported", _settings->already_imported );
    KConfigGroup dataGroup( &cfg,  "Data" );
    int cnt = write_actions( dataGroup, _settings->actions(), true );
    mainGroup.writeEntry( "Autostart", cnt != 0 && !_settings->isDaemonDisabled() );
    mainGroup.writeEntry( "Disabled", _settings->isDaemonDisabled());
    KConfigGroup gesturesConfig( &cfg, "Gestures" );
    gesturesConfig.writeEntry( "Disabled", _settings->areGesturesDisabled() );
    gesturesConfig.writeEntry( "MouseButton", _settings->gestureMouseButton() );
    gesturesConfig.writeEntry( "Timeout", _settings->gestureTimeOut() );
    if( _settings->gesturesExclude() != NULL )
        {
        KConfigGroup gesturesExcludeConfig( &cfg, "GesturesExclude" );
        _settings->gesturesExclude()->cfg_write( gesturesExcludeConfig );
        }
    else
        cfg.deleteGroup( "GesturesExclude" );
    KConfigGroup voiceConfig( &cfg, "Voice" );
    voiceConfig.writeEntry("Shortcut" , _settings->voice_shortcut.toString() );
    }


// return value means the number of enabled actions written in the cfg file
// i.e. 'Autostart' for value > 0 should be on
int SettingsWriter::write_actions(KConfigGroup& cfg_P, const ActionDataGroup* parent_P, bool enabled_P)
    {
    int enabled_cnt = 0;
    QString save_cfg_group = cfg_P.name();
    int cnt = 0;
    if( parent_P )
        {
        Q_FOREACH(ActionDataBase *child,parent_P->children())
            {
            ++cnt;
            if( enabled_P && child->enabled( true ))
                ++enabled_cnt;
            KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( cnt ));
            child->cfg_write( itConfig );
            ActionDataGroup* grp = dynamic_cast< ActionDataGroup* >(child);
            if( grp != NULL )
                enabled_cnt += write_actions( itConfig, grp, enabled_P && child->enabled( true ));
            }
        }
    cfg_P.writeEntry( "DataCount", cnt );
    return enabled_cnt;
    }




} // namespace KHotKeys

