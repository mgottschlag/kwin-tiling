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
#ifndef LAUNCHERITEM_H
#define LAUNCHERITEM_H

#include "abstractgroupableitem.h"
#include "taskmanager_export.h"

// KDE
#include <KDE/KIcon>
#include <KDE/KUrl>


namespace TaskManager
{

class TASKMANAGER_EXPORT LauncherItem : public AbstractGroupableItem
{
    Q_OBJECT
public:
    /** Creates a LauncherItem for a executable*/
    LauncherItem(QObject *parent, const KUrl &url);
    ~LauncherItem();

    /**
    * @deprecated: use itemType() instead
    **/
    KDE_DEPRECATED bool isGroupItem() const;
    ItemType itemType() const;

    KUrl url() const;
    QIcon icon() const;
    QString name() const;
    QString genericName() const;

    void setUrl(const KUrl &url);
    void setIcon(const QIcon &icon);
    void setName(const QString &name);
    void setGenericName(const QString &genericName);

    //reimplemented pure virtual methods from abstractgroupableitem
    bool isOnCurrentDesktop() const;
    bool isOnAllDesktops() const;
    int desktop() const;
    bool isShaded() const;
    bool isMaximized() const;
    bool isMinimized() const;
    bool isFullScreen() const;
    bool isKeptBelowOthers() const;
    bool isAlwaysOnTop() const;
    bool isActionSupported(NET::Action) const;
    bool isActive() const;
    bool demandsAttention() const;
    void addMimeData(QMimeData *) const;

public Q_SLOTS:
    void toDesktop(int);

    void setShaded(bool);
    void toggleShaded();

    void setMaximized(bool);
    void toggleMaximized();

    void setMinimized(bool);
    void toggleMinimized();

    void setFullScreen(bool);
    void toggleFullScreen();

    void setKeptBelowOthers(bool);
    void toggleKeptBelowOthers();

    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();

    void close();

    void launch();

private:
    class Private;
    Private * const d;

};

} // TaskManager namespace

#endif
