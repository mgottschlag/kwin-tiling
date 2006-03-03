/*****************************************************************

Copyright (c) 1996-2003 the kicker authors. See file AUTHORS.

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

#ifndef __container_button_h__
#define __container_button_h__

#include <klocale.h>
#include <kservice.h>
#include <kurl.h>

#include "container_base.h"
//Added by qt3to4:
#include <QPixmap>
#include <QEvent>
#include <QMenu>

class QLayout;
class PanelButton;
class KConfigGroup;

class ButtonContainer : public BaseContainer
{
    Q_OBJECT

public:
    ButtonContainer(QMenu* opMenu, QWidget* parent = 0);

    virtual bool isValid() const;
    virtual bool isAMenu() const { return false; }

    virtual int widthForHeight(int height) const;
    virtual int heightForWidth(int width)  const;

    bool eventFilter (QObject *, QEvent *);
    virtual void completeMoveOperation();

    PanelButton* button() const { return _button; }

public Q_SLOTS:
    void setPopupDirection(Plasma::Position d);
    void setOrientation(Qt::Orientation o);

protected Q_SLOTS:
    void slotMenuClosed();
    void removeRequested();
    void hideRequested(bool);
    void dragButton(const KUrl::List urls, const QPixmap icon);
    void dragButton(const QPixmap icon);

protected:
    virtual void doSaveConfiguration( KConfigGroup&, bool layoutOnly ) const;
    void embedButton(PanelButton* p);
    QMenu* createOpMenu();
    void checkImmutability(const KConfigGroup&);

protected:
    PanelButton  *_button;
    QLayout      *_layout;
    QPoint        _oldpos;
};

class KMenuButtonContainer : public ButtonContainer
{
public:
    KMenuButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    KMenuButtonContainer(QMenu* opMenu, QWidget* parent = 0);
    virtual QString appletType() const { return "KMenuButton"; }
    virtual QString icon() const { return "kmenu"; }
    virtual QString visibleName() const { return i18n("K Menu"); }

    virtual int heightForWidth( int width )  const;
    bool isAMenu() const { return true; }
};

class DesktopButtonContainer : public ButtonContainer
{
public:
    DesktopButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    DesktopButtonContainer(QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "DesktopButton"; }
    virtual QString icon() const { return "desktop"; }
    virtual QString visibleName() const { return i18n("Desktop Access"); }
};

class ServiceButtonContainer : public ButtonContainer
{
public:
    ServiceButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    ServiceButtonContainer(const KService::Ptr & service,  QMenu* opMenu,QWidget* parent = 0);
    ServiceButtonContainer(const QString& desktopFile,  QMenu* opMenu,QWidget* parent = 0);
    QString appletType() const { return "ServiceButton"; }
    virtual QString icon() const;
    virtual QString visibleName() const;
};

class URLButtonContainer : public ButtonContainer
{
public:
    URLButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    URLButtonContainer(const QString& url, QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "URLButton"; }
    virtual QString icon() const;
    virtual QString visibleName() const;
};

class BrowserButtonContainer : public ButtonContainer
{
public:
    BrowserButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    BrowserButtonContainer(const QString& startDir, QMenu* opMenu, const QString& icon = "kdisknav", QWidget* parent = 0);
    QString appletType() const { return "BrowserButton"; }
    virtual QString icon() const { return "kdisknav"; }
    virtual QString visibleName() const { return i18n("Quick Browser"); }
    bool isAMenu() const { return true; }
};

class ServiceMenuButtonContainer : public ButtonContainer
{
public:
    ServiceMenuButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    ServiceMenuButtonContainer(const QString& relPath, QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "ServiceMenuButton"; }
    virtual QString icon() const;
    virtual QString visibleName() const;
    bool isAMenu() const { return true; }
};

class WindowListButtonContainer : public ButtonContainer
{
public:
    WindowListButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    WindowListButtonContainer(QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "WindowListButton"; }
    virtual QString icon() const { return "window_list"; }
    virtual QString visibleName() const { return i18n("Windowlist"); }
    bool isAMenu() const { return true; }
};

class BookmarksButtonContainer : public ButtonContainer
{
public:
    BookmarksButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget* parent = 0);
    BookmarksButtonContainer(QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "BookmarksButton"; }
    virtual QString icon() const { return "bookmark"; }
    virtual QString visibleName() const { return i18n("Bookmarks"); }
    bool isAMenu() const { return true; }
};

class NonKDEAppButtonContainer : public ButtonContainer
{
public:
    NonKDEAppButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget *parent=0);
    NonKDEAppButtonContainer(const QString &name, const QString &description,
                             const QString &filePath, const QString &icon,
                             const QString &cmdLine, bool inTerm,
                             QMenu* opMenu, QWidget* parent = 0);
    QString appletType() const { return "ExecButton"; }
    virtual QString icon() const { return "exec"; }
    virtual QString visibleName() const { return i18n("Non-KDE Application"); }
};

class ExtensionButtonContainer : public ButtonContainer
{
public:
    ExtensionButtonContainer(const KConfigGroup& config, QMenu* opMenu, QWidget *parent=0);
    ExtensionButtonContainer(const QString& desktopFile, QMenu* opMenu, QWidget *parent= 0);
    QString appletType() const { return "ExtensionButton"; }
    virtual QString icon() const;
    virtual QString visibleName() const;
    bool isAMenu() const { return true; }
};

#endif

