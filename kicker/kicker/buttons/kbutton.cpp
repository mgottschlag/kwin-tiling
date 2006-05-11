/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <QToolTip>

#include <klocale.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

#include "kickerSettings.h"

#include "config.h"

#include "menumanager.h"
#include "k_mnu.h"

#include "kbutton.h"
#include "kbutton.moc"

KButton::KButton( QWidget* parent )
    : PanelPopupButton( parent, "KButton" )
{
    this->setToolTip( i18n("Applications, tasks and desktop sessions"));
    setTitle(i18n("K Menu"));

    setPopup(MenuManager::self()->kmenu());
    MenuManager::self()->registerKButton(this);
    setIcon("kmenu");

    if (KickerSettings::showKMenuText())
    {
        setButtonText(KickerSettings::kMenuText());
        setFont(KickerSettings::buttonFont());
        setTextColor(KickerSettings::buttonTextColor());
    }
}

KButton::~KButton()
{
    MenuManager::self()->unregisterKButton(this);
}

void KButton::properties()
{
    KToolInvocation::startServiceByDesktopName("kmenuedit", QStringList(),
                                            0, 0, 0, "", true);
}

void KButton::initPopup()
{
//    kDebug(1210) << "KButton::initPopup()" << endl;

    // this hack is required to ensure the correct popup position
    // when the size of the recent application part of the menu changes
    // please don't remove this _again_
    MenuManager::self()->kmenu()->initialize();
}

