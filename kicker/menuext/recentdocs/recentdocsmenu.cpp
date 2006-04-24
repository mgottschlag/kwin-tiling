/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <QMenuItem>
#include <QMouseEvent>

#include <kglobal.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <k3urldrag.h>
#include <krecentdocument.h>
#include <kmenu.h>
#include <kworkspace.h>
#include <kdedesktopmimetype.h>
#include "recentdocsmenu.h"

K_EXPORT_KICKER_MENUEXT(recentdocs, RecentDocsMenu)

RecentDocsMenu::RecentDocsMenu(QWidget *parent, const char *name,
                               const QStringList &/*args*/)
    : KPanelMenu(KRecentDocument::recentDocumentDirectory(), parent, name)
{
}

RecentDocsMenu::~RecentDocsMenu()
{
}

void RecentDocsMenu::initialize() {
	if (initialized()) clear();

	insertItem(SmallIconSet("history_clear"), i18n("Clear History"),
	           this, SLOT(slotClearHistory()));
	insertSeparator();

	_fileList = KRecentDocument::recentDocuments();

	if (_fileList.isEmpty()) {
		insertItem(i18n("No Entries"), 0);
		setItemEnabled(0, false);
		return;
    }

	int id = 0;

	for (QStringList::ConstIterator it = _fileList.begin();
	     it != _fileList.end();
	     ++it)
	{
		KDesktopFile f(*it, true /* read only */);
		insertItem(SmallIconSet(f.readIcon()), f.readName().replace('&', QString::fromAscii("&&") ), id++);
    }

    setInitialized(true);
}

void RecentDocsMenu::slotClearHistory() {
    KRecentDocument::clear();
    reinitialize();
}

void RecentDocsMenu::slotExec(int id) {
	if (id >= 0) {
		KWorkSpace::propagateSessionManager();
		KUrl u;
		u.setPath(_fileList[id]);
		KDEDesktopMimeType::run(u, true);
	}
}

void RecentDocsMenu::mousePressEvent(QMouseEvent* e) {
	_mouseDown = e->pos();
	KPanelMenu::mousePressEvent(e);
}

void RecentDocsMenu::mouseMoveEvent(QMouseEvent* e) {
	KPanelMenu::mouseMoveEvent(e);

	if (!(e->state() & Qt::LeftButton))
		return;

	if (!rect().contains(_mouseDown))
		return;

	int dragLength = (e->pos() - _mouseDown).manhattanLength();

	if (dragLength <= KGlobalSettings::dndEventDelay())
		return;  // ignore it

	int id = static_cast<QMenuItem*>(actionAt(_mouseDown))->id();

	// Don't drag 'manual' items.
	if (id < 0)
		return;

	KDesktopFile f(_fileList[id], true /* read only */);

	KUrl url ( f.readURL() );

	if (url.isEmpty()) // What are we to do ?
		return;

	KUrl::List lst;
	lst.append(url);

	K3URLDrag* d = new K3URLDrag(lst, this);
	d->setPixmap(SmallIcon(f.readIcon()));
	d->dragCopy();
	close();
}

void RecentDocsMenu::slotAboutToShow()
{
    reinitialize();
    KPanelMenu::slotAboutToShow();
}

#include "recentdocsmenu.moc"
