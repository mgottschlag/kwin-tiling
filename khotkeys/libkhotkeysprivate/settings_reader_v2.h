#ifndef SETTINGS_READER_V2_H
#define SETTINGS_READER_V2_H
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

#include "action_data/action_data_visitor.h"

class KConfigBase;
class KConfigGroup;

namespace KHotKeys {
    class Settings;
    template< typename T, typename A > class SimpleActionDataHelper;
}


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class SettingsReaderV2 : public KHotKeys::ActionDataVisitor
    {
public:
    SettingsReaderV2(
            KHotKeys::Settings *settings,
            bool loadAll=true,
            bool loadDisabled=true);

    virtual ~SettingsReaderV2();

    void read(const KConfigBase &config, KHotKeys::ActionDataGroup *parent);

    KHotKeys::ActionDataGroup *readGroup(const KConfigGroup &config, KHotKeys::ActionDataGroup *parent);

    KHotKeys::ActionDataBase *readAction(const KConfigGroup &config, KHotKeys::ActionDataGroup *parent);

    virtual void visitActionDataBase(KHotKeys::ActionDataBase *base);

    virtual void visitActionData(KHotKeys::ActionData *group);

    virtual void visitActionDataGroup(KHotKeys::ActionDataGroup *group);

    virtual void visitGenericActionData(KHotKeys::Generic_action_data *data);

    virtual void visitMenuentryShortcutActionData(KHotKeys::MenuEntryShortcutActionData *data);

    virtual void visitSimpleActionData(KHotKeys::SimpleActionData *data);

private:

    const KConfigGroup *_config;

    KHotKeys::Settings *_settings;

    bool _loadAll;

    bool _disableActions;

    }; // SettingsReaderV2


#endif /* SETTINGS_READER_V2_H */

