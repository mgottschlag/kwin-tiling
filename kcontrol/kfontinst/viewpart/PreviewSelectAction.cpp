/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "PreviewSelectAction.h"
#include "UnicodeBlocks.h"
#include "UnicodeScripts.h"
#include <klocale.h>

namespace KFI
{

CPreviewSelectAction::CPreviewSelectAction(QObject *parent, bool all)
                    : KSelectAction(KIcon("character-set"), i18n("Preview Type"), parent),
                      itsNumUnicodeBlocks(0)
{
    QStringList items;
    items.append(i18n("Standard Preview"));
    items.append(i18n("All Characters"));

    if(all)
    {
        for(itsNumUnicodeBlocks=0; constUnicodeBlocks[itsNumUnicodeBlocks].blockName; ++itsNumUnicodeBlocks)
            items.append(i18n("Unicode Block: %1", i18n(constUnicodeBlocks[itsNumUnicodeBlocks].blockName)));

        for(int i=0; constUnicodeScriptList[i]; ++i)
            items.append(i18n("Unicode Script: %1", i18n(constUnicodeScriptList[i])));
    }
    else
        for(int i=0; constUnicodeScriptList[i]; ++i)
            items.append(i18n(constUnicodeScriptList[i]));

    setItems(items);
    setCurrentItem(0);

    connect(this, SIGNAL(triggered(int)), SLOT(selected(int)));
}

void CPreviewSelectAction::setStd()
{
    setCurrentItem(0);
    selected(0);
}

void CPreviewSelectAction::selected(int index)
{
    QList<CFcEngine::TRange> list;

    if(0==index)
        ;
    else if(1==index)
        list.append(CFcEngine::TRange());
    else if(index<itsNumUnicodeBlocks+2)
        list.append(CFcEngine::TRange(constUnicodeBlocks[index-2].start, constUnicodeBlocks[index-2].end));
    else
    {
        int script(index-(2+itsNumUnicodeBlocks));

        for(int i=0; constUnicodeScripts[i].scriptIndex>=0; ++i)
            if(constUnicodeScripts[i].scriptIndex==script)
                list.append(CFcEngine::TRange(constUnicodeScripts[i].start, constUnicodeScripts[i].end));
    }

    emit range(list);
}

}

#include "PreviewSelectAction.moc"
