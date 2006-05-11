/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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

#include <QDir>

#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kurl.h>
#include <kauthorized.h>

#include "browser_mnu.h"
#include "quickbrowser_mnu.h"
#include "quickbrowser_mnu.moc"

PanelQuickBrowser::PanelQuickBrowser(QWidget *parent)
  : KPanelMenu("", parent) {}

void PanelQuickBrowser::initialize()
{
    if(initialized()) return;
    setInitialized(true);

    KUrl url;
    
    url.setPath(QDir::homePath());
    if (KAuthorized::authorizeURLAction("list", KUrl(), url))
        insertItem(SmallIcon("kfm_home"), i18n("&Home Folder"),
               new PanelBrowserMenu(url.path(), this));
               
    url.setPath(QDir::rootPath());
    if (KAuthorized::authorizeURLAction("list", KUrl(), url))
        insertItem(SmallIcon("folder_red"), i18n("&Root Folder"),
               new PanelBrowserMenu(url.path(), this));
               
    url.setPath(QDir::rootPath() + "etc");
    if (KAuthorized::authorizeURLAction("list", KUrl(), url))
        insertItem(SmallIcon("folder_yellow"), i18n("System &Configuration"),
               new PanelBrowserMenu(url.path(), this));
}
