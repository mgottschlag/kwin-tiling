/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _INFO_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "info_tab.h"

#include <QPushButton>
#include <QLabel>

#include <klocale.h>

namespace KHotKeys
{

Info_tab::Info_tab( QWidget* parent_P, const char* name_P )
    : Info_tab_ui( parent_P, name_P )
    {
    clear_data();
    }

void Info_tab::clear_data()
    {
    // "global" tab, not action specific, do nothing
    }

} // namespace KHotKeys

#include "info_tab.moc"
