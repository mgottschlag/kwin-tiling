/*****************************************************************

Copyright (c) 2000 Matthias Elter

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

#ifndef __dockbarextension_h__
#define __dockbarextension_h__

#include <QVector>
#include <kpanelextension.h>
#include "dockcontainer.h"
#include "utils.h"

class KWinModule;

class DockBarExtension : public KPanelExtension
{
    Q_OBJECT

public:
    DockBarExtension(const QString& configFile, int actions = 0, QWidget *parent = 0);

    virtual ~DockBarExtension();

    QSize sizeHint(Plasma::Position, QSize maxSize) const;
    Plasma::Position preferedPosition() const { return Plasma::Right; }

protected Q_SLOTS:
    void windowAdded(WId);
    void embeddedWindowDestroyed(DockContainer*);
    void settingsChanged(DockContainer*);

protected:
    void resizeEvent(QResizeEvent*);
    void embedWindow(WId win, QString command, QString resName, QString resClass);
    void addContainer(DockContainer*, int pos=-1);
    void removeContainer(DockContainer*);
    void saveContainerConfig();
    void loadContainerConfig();
    void layoutContainers();
    int findContainerAtPoint(const QPoint&);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
private:
    KWinModule* kwin_module;
    DockContainer::Vector containers;

    // handle the dragging of applets
    DockContainer *dragging_container, *original_container;
    QPoint mclic_pos, mclic_dock_pos;
    int dragged_container_original_pos;
};

#endif
