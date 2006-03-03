/*****************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qtooltip.h>

#include <kconfig.h>
#include <kservicegroup.h>
#include <klocale.h>

#include "service_mnu.h"

#include "servicemenubutton.h"
#include "servicemenubutton.moc"

ServiceMenuButton::ServiceMenuButton( const QString& relPath, QWidget* parent )
  : PanelPopupButton( parent, "ServiceMenuButton" )
  , topMenu( 0 )
{
    initialize( relPath );
}

ServiceMenuButton::ServiceMenuButton( const KConfigGroup& config, QWidget* parent )
  : PanelPopupButton( parent, "ServiceMenuButton" )
  , topMenu( 0 )
{
    initialize( config.readPathEntry("RelPath") );
}

void ServiceMenuButton::initialize( const QString& relPath )
{
    KServiceGroup::Ptr group = KServiceGroup::group( relPath );

    if (!group || !group->isValid())
    {
        setIsValid(false);
        return;
    }

    QString caption = group->caption();
    if (caption.isEmpty())
    {
        caption = i18n("Applications");
    }

    QString comment = group->comment();
    if (comment.isEmpty())
    {
        comment = caption;
    }

    topMenu = new PanelServiceMenu(caption, relPath);
    setPopup(topMenu);
    this->setToolTip( comment);
    setTitle(caption);
    setIcon(group->icon());
}

void ServiceMenuButton::saveConfig( KConfigGroup& config ) const
{
    if (topMenu)
        config.writePathEntry("RelPath", topMenu->relPath());
}

void ServiceMenuButton::initPopup()
{
    if( !topMenu->initialized() ) {
        topMenu->reinitialize();
    }
}

void ServiceMenuButton::startDrag()
{
    KUrl url("programs:/" + topMenu->relPath());
    emit dragme(KUrl::List(url), labelIcon());
}

