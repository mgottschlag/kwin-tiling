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

#include <KDebug>
#include <KDesktopFile>
#include <KMimeType>
#include <QMimeData>
#include <KRun>

#include "taskitem.h"
#include "taskgroup.h"

namespace TaskManager
{

class LauncherItemPrivate
{
public:
    LauncherItemPrivate(LauncherItem *launcher)
        : q(launcher)
    {
    }

    void associateDestroyed(QObject *obj);

    LauncherItem *q;
    KUrl        url;
    QIcon       icon;
    QString     name;
    QString     genericName;
    QSet<QObject *> associates;
};

LauncherItem::LauncherItem(QObject *parent, const KUrl &url)
    : AbstractGroupableItem(parent),
    d(new LauncherItemPrivate(this))
{
    d->genericName = QString();
    d->name = QString();
    if (url.isEmpty()) {
        d->icon = KIcon("unknown");
    } else {
        setLauncherUrl(url);
    }
}

LauncherItem::~LauncherItem()
{
    emit destroyed(this);
    delete d;
}

void LauncherItem::associateItemIfMatches(AbstractGroupableItem *item)
{
    if (d->associates.contains(item)) {
        return;
    }

    QString name;
    if (item->itemType() == TaskItemType && !item->isStartupItem()) {
        name = static_cast<TaskItem *>(item)->task()->classClass().toLower();
    } else {
        name = item->name().toLower();
    }

    if (name.isEmpty()) {
        return;
    }

    if (name.compare(d->name, Qt::CaseInsensitive) == 0) {
        const bool wasEmpty = d->associates.isEmpty();

        d->associates.insert(item);
        connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(associateDestroyed(QObject*)));

        if (wasEmpty) {
            emit show(true);
        }
    }
}

void LauncherItem::removeItemIfAssociated(AbstractGroupableItem *item)
{
    disconnect(item, SIGNAL(destroyed(QObect*)), this, SLOT(associateDestroyed(QObject*)));

    // now let's just pretend it was destroyed
    d->associateDestroyed(item);
}

bool LauncherItem::shouldShow() const
{
    return d->associates.isEmpty();
}

void LauncherItemPrivate::associateDestroyed(QObject *obj)
{
    kDebug() << "associate was destroyed" << associates.isEmpty();
    if (associates.isEmpty()) {
        return;
    }

    associates.remove(obj);

    if (associates.isEmpty()) {
        emit q->show(false);
    }
}

QIcon LauncherItem::icon() const
{
    return d->icon;
}

QString LauncherItem::name() const
{
    return d->name;
}

QString LauncherItem::genericName() const
{
    return d->genericName;
}

void LauncherItem::setName(const QString& name)
{
    d->name = name;
}

void LauncherItem::setGenericName(const QString& genericName)
{
    d->genericName = genericName;
}

ItemType LauncherItem::itemType() const
{
    return LauncherItemType;
}

bool LauncherItem::isGroupItem() const
{
    return false;
}

void LauncherItem::launch()
{
    new KRun(d->url, 0);
}

void LauncherItem::addMimeData(QMimeData* mimeData ) const
{
    mimeData->setData("text/uri-list", d->url.url().toAscii());
}

KUrl LauncherItem::launcherUrl() const
{
    return d->url;
}

void LauncherItem::setLauncherUrl(const KUrl &url)
{
    // Takes care of improperly escaped characters and resolves paths
    // into file:/// URLs
    KUrl newUrl(url.url());

    if (newUrl == d->url) {
        return;
    }

    d->url = newUrl;

    if (d->url.isLocalFile() && KDesktopFile::isDesktopFile(d->url.toLocalFile())) {
        KDesktopFile f(d->url.toLocalFile());

        d->icon = KIcon(f.readIcon());
        d->name = f.readName();
        d->genericName = f.readGenericName();
    } else {
        d->icon = KIcon(KMimeType::iconNameForUrl(d->url));
    }

    if (d->name.isEmpty()) {
        d->name = d->url.fileName();
    }

    if (d->icon.isNull()) {
        d->icon = KIcon("unknown");
    }
}

void LauncherItem::setIcon(const QIcon& icon)
{
    d->icon = icon;
}

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
