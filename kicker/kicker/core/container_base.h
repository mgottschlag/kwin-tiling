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

#ifndef __container_base_h__
#define __container_base_h__

#include <QWidget>
#include <QPoint>
#include <QList>

#include <kpanelapplet.h>
#include <kpanelextension.h>

class KConfigGroup;
class QMenu;
class QMimeType;

class BaseContainer : public QWidget
{
    Q_OBJECT

public:
    typedef QList<BaseContainer*> List;
    typedef QList<BaseContainer*>::iterator Iterator;
    typedef QList<BaseContainer*>::const_iterator ConstIterator;

    BaseContainer( QMenu* appletOpMenu, QWidget* parent = 0, const char * name = 0 );
    ~BaseContainer();

    virtual int widthForHeight(int height) const = 0;
    virtual int heightForWidth(int width)  const = 0;

    virtual bool isStretch() const { return false; }

    virtual void completeMoveOperation() {}
    virtual void about() {}
    virtual void help() {}
    virtual void preferences() {}
    virtual void reportBug() {}

    virtual bool isValid() const { return true; }
    virtual bool isImmutable() const;
    virtual void setImmutable(bool immutable) { m_immutable = immutable; }

    double freeSpace() const { return _fspace; }
    void setFreeSpace(double f) { _fspace = f; }

    QString appletId() const { return _aid; }
    void setAppletId(const QString& s) { _aid = s; }

    virtual int actions() const { return _actions; }

    Plasma::Position popupDirection() const { return _dir; }
    Qt::Orientation orientation() const { return _orient; }
    Plasma::Alignment alignment() const { return _alignment; }

    QMenu* opMenu();

    void loadConfiguration( KConfigGroup& );
    void saveConfiguration( KConfigGroup&, bool layoutOnly = false ) const;

    void configure(Qt::Orientation, Plasma::Position);
    virtual void configure() {}

    QPoint moveOffset() const { return _moveOffset; }

    virtual QString appletType() const = 0;
    virtual QString icon() const { return "unknown"; }
    virtual QString visibleName() const = 0;

    void populateMimeData(QMimeData* mimeData);
    static bool canDecode(const QMimeData* mimeData);
    static BaseContainer* fromMimeData(const QMimeData* mimeData);

public Q_SLOTS:
    virtual void slotRemoved(KConfig* config);
    virtual void setPopupDirection(Plasma::Position d) { _dir = d; }
    virtual void setOrientation(Qt::Orientation o) { _orient = o; }

    void setAlignment(Plasma::Alignment a);

Q_SIGNALS:
    void removeme(BaseContainer*);
    void takeme(BaseContainer*);
    void moveme(BaseContainer*);
    void maintainFocus(bool);
    void requestSave();
    void focusReqested(bool);

protected:
    virtual void changeEvent(QEvent* e);
    virtual void doLoadConfiguration( KConfigGroup& ) {}
    virtual void doSaveConfiguration( KConfigGroup&,
                                      bool /* layoutOnly */ ) const {}
    virtual void alignmentChange(Plasma::Alignment) {}

    virtual QMenu* createOpMenu() = 0;
    QMenu *appletOpMenu() const { return _appletOpMnu; }

    Plasma::Position   _dir;
    Qt::Orientation    _orient;
    Plasma::Alignment  _alignment;
    double             _fspace;
    QPoint             _moveOffset;
    QString            _aid;
    int                _actions;
    bool               m_immutable;

private:
    QMenu        *_opMnu;
    QMenu        *_appletOpMnu;
};

#endif

