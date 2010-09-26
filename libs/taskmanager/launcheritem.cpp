/*****************************************************************

Copyright 2010 Anton Kreuzkamp <akreuzkamp@web.de>

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
// Own
#include "launcheritem.h"

#include <KDE/KDesktopFile>
#include <KDE/KMimeType>
#include <KDE/KRun>
#include "taskgroup.h"


namespace TaskManager
{

class LauncherItem::Private
{
public:
    KUrl        url;
    QIcon       icon;
    QString     name;
    int         windowInstances;
};

LauncherItem::LauncherItem(QObject *parent, const KUrl &url)
    : AbstractGroupableItem(parent),
    d(new Private)
{
    if (!url.isEmpty()) {
        setUrl(url);
    } else {
        d->icon = KIcon("unknown");
    }
    d->windowInstances = 0;
}

LauncherItem::~LauncherItem()
{
    emit destroyed(this);
    delete d;
}

QIcon LauncherItem::icon() const
{
    return d->icon;
}

QString LauncherItem::name() const
{
    return d->name;
}

ItemType LauncherItem::itemType() const
{
    return LauncherItemType;
}

bool LauncherItem::isGroupItem() const
{
    return false;
}

void LauncherItem::setUrl(const KUrl &url)
{
    // Takes care of improperly escaped characters and resolves paths
    // into file:/// URLs
    KUrl newUrl(url.url());

    if (newUrl == d->url) {
        return;
    }

    d->url = newUrl;

    if (d->url.isLocalFile() &&
        KDesktopFile::isDesktopFile(d->url.toLocalFile())) {
        KDesktopFile f(d->url.toLocalFile());

        d->icon = KIcon(f.readIcon());
        d->name = f.readName();
    } else {
        d->icon = KIcon(KMimeType::iconNameForUrl(d->url));
    }

    if (d->name.isNull()) {
        d->name = d->url.fileName();
    }

    if (d->icon.isNull()) {
        d->icon = KIcon("unknown");
    }
}

void LauncherItem::execute()
{
    new KRun(d->url, 0);
}

void LauncherItem::addWindowInstance()
{
    d->windowInstances++;
    if (parentGroup() && d->windowInstances == 1) {
        parentGroup()->launcherStatusChanged(this);
    }
}

void LauncherItem::removeWindowInstance()
{
    d->windowInstances--;
    if (parentGroup() && d->windowInstances == 0) {
        parentGroup()->launcherStatusChanged(this);
    }
}

bool LauncherItem::isVisible() const
{
    return d->windowInstances == 0;
}

//BEGIN reimplemented pure virtual methods from abstractgroupableitem
bool LauncherItem::demandsAttention() const
{
    return false;
}
bool LauncherItem::isActionSupported(NET::Action ) const
{
    return false;
}
bool LauncherItem::isActive() const
{
    return false;
}
bool LauncherItem::isAlwaysOnTop() const
{
    return false;
}
bool LauncherItem::isFullScreen() const
{
    return false;
}
bool LauncherItem::isKeptBelowOthers() const
{
    return false;
}
bool LauncherItem::isMaximized() const
{
    return false;
}
bool LauncherItem::isMinimized() const
{
    return false;
}
bool LauncherItem::isOnAllDesktops() const
{
    return false;
}
bool LauncherItem::isOnCurrentDesktop() const
{
    return false;
}
bool LauncherItem::isShaded() const
{
    return false;
}
int LauncherItem::desktop() const
{
    return 0;
}
void LauncherItem::addMimeData(QMimeData* ) const
{
}

void LauncherItem::setAlwaysOnTop(bool )
{
}
void LauncherItem::setFullScreen(bool )
{
}
void LauncherItem::setKeptBelowOthers(bool )
{
}
void LauncherItem::setMaximized(bool )
{
}
void LauncherItem::setMinimized(bool )
{
}
void LauncherItem::setShaded(bool )
{
}
void LauncherItem::toDesktop(int )
{
}
void LauncherItem::toggleAlwaysOnTop()
{
}
void LauncherItem::toggleFullScreen()
{
}
void LauncherItem::toggleKeptBelowOthers()
{
}
void LauncherItem::toggleMaximized()
{
}
void LauncherItem::toggleMinimized()
{
}
void LauncherItem::toggleShaded()
{
}
void LauncherItem::close()
{
}
//END


} // TaskManager namespace

#include "launcheritem.moc"
