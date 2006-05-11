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

#include <kactioncollection.h>
#include <kbookmark.h>
#include <kbookmarkmenu.h>
#include <konqbookmarkmanager.h>
#include <klocale.h>
#include <kmenu.h>

#include "bookmarksbutton.h"
#include "bookmarksbutton.moc"

BookmarksButton::BookmarksButton(QWidget* parent)
    : PanelPopupButton(parent, "BookmarksButton"),
      bookmarkParent(0),
      bookmarkMenu(0),
      actionCollection(0),
      bookmarkOwner(0)
{
    actionCollection = new KActionCollection( this );
    bookmarkParent = new KMenu(this);
    bookmarkParent->setObjectName("bookmarks");
    bookmarkOwner = new KBookmarkOwner;
    bookmarkMenu = new KBookmarkMenu(KonqBookmarkManager::self(),
                                     bookmarkOwner,
                                     bookmarkParent,
                                     actionCollection,
                                     true, false);
    setPopup(bookmarkParent);
    this->setToolTip( i18n("Bookmarks"));
    setTitle(i18n("Bookmarks"));
    setIcon("bookmark");
}

BookmarksButton::~BookmarksButton()
{
    delete bookmarkMenu;
    delete bookmarkOwner;
}

void BookmarksButton::initPopup()
{
    bookmarkMenu->ensureUpToDate();
}

void BookmarksButton::properties()
{
    KonqBookmarkManager::self()->slotEditBookmarks();
}

