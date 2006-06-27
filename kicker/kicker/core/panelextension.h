/*****************************************************************

Copyright (c) 2000 Matthias Elter
              2004 Aaron J. Seigo <aseigo@kde.org>

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

#ifndef _panelextension_h_
#define _panelextension_h_

#include <kpanelextension.h>

#include "appletinfo.h"

#include <QGridLayout>
#include <QEvent>
#include <QMenu>

class AppletContainer;
class ContainerArea;
class QMenu;
class QGridLayout;

// This is the KPanelExtension responsible for the main kicker panel
// Prior to KDE 3.4 it was the ChildPanelExtension

class PanelExtension : public KPanelExtension, virtual public  DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    PanelExtension(const QString& configFile, QWidget *parent = 0);
    virtual ~PanelExtension();

    QMenu* opMenu();

k_dcop:
    int panelSize() { return sizeInPixels(); }
    int panelOrientation() { return static_cast<int>(orientation()); }
    int panelPosition() { return static_cast<int>(position()); }

    void setPanelSize(int size);
    void addKMenuButton();
    void addDesktopButton();
    void addWindowListButton();
    void addURLButton(const QString &url);
    void addBrowserButton(const QString &startDir);
    void addServiceButton(const QString &desktopEntry);
    void addServiceMenuButton(const QString &name, const QString& relPath);
    void addNonKDEAppButton(const QString &filePath, const QString &icon,
                            const QString &cmdLine, bool inTerm);
    void addNonKDEAppButton(const QString &title, const QString &description,
                            const QString &filePath, const QString &icon,
                            const QString &cmdLine, bool inTerm);

    void addApplet(const QString &desktopFile);

public:
    QSize sizeHint(Plasma::Position, QSize maxSize) const;
    Plasma::Position preferedPosition() const { return Plasma::Bottom; }
    bool eventFilter( QObject *, QEvent * );

protected:
    void positionChange(Plasma::Position);

    ContainerArea    *_containerArea;

protected Q_SLOTS:
    void configurationChanged();
    void immutabilityChanged(bool);
    void slotBuildOpMenu();
    void showConfig();
    virtual void populateContainerArea();

private:
    QMenu* m_opMenu;
    QMenu* m_panelAddMenu;
    QMenu* m_removeMenu;
    QMenu* m_addExtensionMenu;
    QMenu* m_removeExtensionMenu;
    QString m_configFile;
    bool m_opMenuBuilt;
};

class MenubarExtension : public PanelExtension
{
    Q_OBJECT

    public:
        MenubarExtension(const AppletInfo& info);
        virtual ~MenubarExtension();

    protected Q_SLOTS:
        virtual void populateContainerArea();

    private:
        MenubarExtension();

        AppletContainer* m_menubar;
};

#endif
