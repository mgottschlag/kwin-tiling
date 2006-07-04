/*****************************************************************

Copyright (c) 1996-2004 the kicker authors. See file AUTHORS.

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

#ifndef __containerarea_h__
#define __containerarea_h__

#include <QTimer>
#include <QPixmap>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QDragMoveEvent>
#include <QEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include <appletinfo.h>

#include "utils.h"
#include "panner.h"
#include "container_base.h"

class KConfig;
class DragIndicator;
class PanelContainer;

class AppletContainer;
class ContainerAreaLayout;
class AddAppletDialog;

class ContainerArea : public Panner
{
    Q_OBJECT

public:
    ContainerArea( KConfig* config, QWidget* parent, QMenu* opMenu, const char* name = 0 );
    ~ContainerArea();

    void initialize(bool useDefaultConfig);
    int position() const;
    Plasma::Position popupDirection() const;
    bool isImmutable() const;

    const QWidget* addButton(const AppletInfo& info);
    const QWidget* addKMenuButton();
    const QWidget* addDesktopButton();
    const QWidget* addWindowListButton();
    const QWidget* addBookmarksButton();
    const QWidget* addServiceButton(const QString& desktopFile);
    const QWidget* addURLButton(const QString &url);
    const QWidget* addBrowserButton();
    const QWidget* addBrowserButton(const QString &startDir,
                            const QString& icon = QString("kdisknav"));
    const QWidget* addServiceMenuButton(const QString& relPath);
    const QWidget* addNonKDEAppButton();
    const QWidget* addNonKDEAppButton(const QString &name,
                                      const QString &description,
                                      const QString &filePath,
                                      const QString &icon,
                                      const QString &cmdLine, bool inTerm);
    const QWidget* addExtensionButton(const QString& desktopFile);
    AppletContainer* addApplet(const AppletInfo& desktopFile,
                               bool isImmutable = false);

    void configure();

    bool inMoveOperation() const { return (_moveAC != 0); }
    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

    BaseContainer::List containers(const QString& type) const;
    int containerCount(const QString& type) const;
    bool canAddContainers() const;

Q_SIGNALS:
    void maintainFocus(bool);

public Q_SLOTS:
    void removeContainer(BaseContainer* a);
    void removeContainers(BaseContainer::List containers);
    void takeContainer(BaseContainer* a);
    void setPosition(Plasma::Position p);
    void setAlignment(Plasma::Alignment a);
    void slotSaveContainerConfig();
    void repaint();
    void showAddAppletDialog();
    void addAppletDialogDone();

protected:
    QString createUniqueId(const QString& appletType) const;
    void completeContainerAddition(BaseContainer* container);

    bool eventFilter(QObject*, QEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dragLeaveEvent(QDragLeaveEvent*);
    void dropEvent(QDropEvent*);
    void viewportResizeEvent(QResizeEvent*);

    void defaultContainerConfig();
    void loadContainers(const QStringList& containers);
    void saveContainerConfig(bool layoutOnly = false);

    QRect availableSpaceFollowing(BaseContainer*);
    void moveDragIndicator(int pos);

    void scrollTo(BaseContainer*);

    void addContainer(BaseContainer* a, bool arrange = false);
    void removeAllContainers();
    void resizeContents();

protected Q_SLOTS:
    void autoScroll();
    void immutabilityChanged(bool);
    void startContainerMove(BaseContainer*);

private:
    BaseContainer::List   m_containers;
    BaseContainer*  _moveAC;
    Plasma::Position _pos;
    KConfig*	    _config;
    DragIndicator*  _dragIndicator;
    BaseContainer*  _dragMoveAC;
    QPoint	    _dragMoveOffset;
    QMenu*          m_opMenu;
    bool            _transparent;
    bool            _useBgTheme;
    bool            _bgSet;
    QTimer          _autoScrollTimer;
    bool            m_canAddContainers;
    bool            m_immutable;

    QWidget*             m_contents;
    ContainerAreaLayout* m_layout;
    AddAppletDialog*     m_addAppletDialog;
};


class DragIndicator : public QWidget
{
    Q_OBJECT

public:
    DragIndicator(QWidget* parent = 0)
    : QWidget(parent) {;}
    ~DragIndicator() {;}

    QSize preferredSize() const { return _preferredSize; }
    void setPreferredSize(const QSize& size) { _preferredSize = size; }

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);

private:
    QSize _preferredSize;
};

#endif

