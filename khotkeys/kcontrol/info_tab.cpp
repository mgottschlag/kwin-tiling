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

#include <qpushbutton.h>
#include <qlabel.h>

#include <klocale.h>

#include <khotkeysglobal.h>

namespace KHotKeys
{

Info_tab::Info_tab( QWidget* parent_P, const char* name_P )
    : Info_tab_ui( parent_P, name_P )
    {
    QString version = i18n( "version %1" ).arg( KHOTKEYS_VERSION );
    version_label->setText( version );
    clear_data();
    }

void Info_tab::clear_data()
    {
    }

} // namespace KHotKeys

#include "info_tab.moc"
