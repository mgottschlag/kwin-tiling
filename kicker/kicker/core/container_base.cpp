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

#include <sys/types.h>
#include <unistd.h>

#include <QMenu>
#include <QMenuItem>

#include <kglobal.h>
#include <kconfig.h>
#include <kdebug.h>

#include "utils.h"
#include "appletop_mnu.h"
#include "kicker.h"

#include "container_base.h"
#include <kconfiggroup.h>
#include "container_base.moc"


BaseContainer::BaseContainer( QMenu* appletOpMenu, QWidget* parent )
  : QWidget( parent )
  , _dir(Plasma::Up)
  , _orient(Qt::Horizontal)
  , _alignment(Plasma::LeftTop)
  , _fspace(0)
  , _moveOffset(QPoint(0,0))
  , _aid(QString())
  , _actions(0)
  , m_immutable(false)
  , _opMnu(0)
  , _appletOpMnu(appletOpMenu)
{}

BaseContainer::~BaseContainer()
{
    delete _opMnu;
}

void BaseContainer::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::ParentChange)
        emit takeme(this);
    QWidget::changeEvent(e);
}

bool BaseContainer::isImmutable() const
{
    return m_immutable || Kicker::self()->isImmutable();
}

void BaseContainer::loadConfiguration( KConfigGroup& group )
{
    setFreeSpace( qMin( group.readEntry( "FreeSpace2", 0.0 ), 1.0 ) );
    doLoadConfiguration( group );
}

void BaseContainer::saveConfiguration(KConfigGroup& group,
                                      bool layoutOnly) const
{
    if (isImmutable())
    {
        return;
    }

    // write positioning info
    group.writeEntry( "FreeSpace2", freeSpace() );
    // write type specific info
    doSaveConfiguration( group, layoutOnly );
}

void BaseContainer::configure(Qt::Orientation o,
                              Plasma::Position d)
{
    setOrientation(o);
    setPopupDirection(d);
    configure();
}

void BaseContainer::populateMimeData(QMimeData* mimeData)
{
    pid_t source_pid = getpid();

    QByteArray a;
    a.resize(sizeof(BaseContainer*) + sizeof(pid_t));
    BaseContainer* ptr = this;
    memcpy(a.data(), &ptr, sizeof(BaseContainer*));
    memcpy(a.data() + sizeof(BaseContainer*), &source_pid, sizeof(pid_t));

    mimeData->setData("application/x-kde-plasma-BaseContainer", a);
}

bool BaseContainer::canDecode(const QMimeData* mimeData)
{
    return mimeData->hasFormat("application/x-kde-plasma-BaseContainer");
}

BaseContainer* BaseContainer::fromMimeData(const QMimeData* mimeData)
{
    QByteArray a = mimeData->data("application/x-kde-plasma-BaseContainer");

    if (a.size() != sizeof(BaseContainer*) + sizeof(pid_t))
    {
        return 0;
    }

    pid_t target_pid = getpid();
    pid_t source_pid;
    memcpy(&source_pid, a.data() + sizeof(BaseContainer*), sizeof(pid_t));

    if (source_pid != target_pid)
    {
        return 0;
    }

    BaseContainer* container;
    memcpy(&container, a.data(), sizeof(BaseContainer*));
    return container;
}

void BaseContainer::slotRemoved(KConfig* config)
{
    if (!config)
    {
        config = KGlobal::config().data();
    }

    config->deleteGroup(appletId().toLatin1());
    config->sync();
}

void BaseContainer::setAlignment(Plasma::Alignment a)
{
    if (_alignment == a)
    {
        return;
    }

    _alignment = a;
    alignmentChange(a);
}

QMenu* BaseContainer::opMenu()
{
    if (_opMnu == 0)
    {
        _opMnu = createOpMenu();
    }

    return Plasma::reduceMenu(_opMnu);
}
