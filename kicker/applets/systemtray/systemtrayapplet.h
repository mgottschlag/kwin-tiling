/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#ifndef __systemtrayapplet_h__
#define __systemtrayapplet_h__

#include <QVector>
#include <QStringList>
#include <QX11EmbedWidget>
#include <QResizeEvent>

#include <kapplication.h>
#include <kpanelapplet.h>

#include "utils.h"

class QTimer;
class KWinModule;
class TrayEmbed;
class KDialog;
class KActionSelector;
class SimpleButton;

class SystemTrayApplet : public KPanelApplet
{
    Q_OBJECT
    typedef QVector<TrayEmbed*> TrayEmbedList;

public:

    SystemTrayApplet(const QString& configFile, Plasma::Type t = Plasma::Normal, int actions = 0,
                     QWidget *parent = 0);
    ~SystemTrayApplet();

    int widthForHeight(int h) const;
    int heightForWidth(int w) const;
    int maxIconWidth() const;
    int maxIconHeight() const;

public slots: //dcop stuff
    void loadSettings();

protected:
    void resizeEvent( QResizeEvent* );
    bool x11Event( XEvent *e );
    void preferences();
    void orientationChange( Qt::Orientation );

protected Q_SLOTS:
    void initialize();
    void systemTrayWindowAdded( WId );
    void updateTrayWindows();
    void layoutTray();
    void toggleExpanded();
    void settingsDialogFinished();
    void applySettings();
    void checkAutoRetract();

private:
    void embedWindow( WId w, bool kde_tray );
    bool isWinManaged( WId w);
    bool shouldHide( WId w);
    void updateVisibleWins();
    void expand();
    void retract();
    void showExpandButton(bool show);
    void refreshExpandButton();

    TrayEmbedList m_shownWins;
    TrayEmbedList m_hiddenWins;
    QStringList m_hiddenIconList;
    QStringList m_sortOrderIconList;
    KWinModule *kwin_module;
    Atom net_system_tray_selection;
    Atom net_system_tray_opcode;
    bool m_showFrame;
    bool m_showHidden;
    SimpleButton *m_expandButton;
    KDialog* m_settingsDialog;
    KActionSelector* m_iconSelector;
    QTimer* m_autoRetractTimer;
    bool m_autoRetract;
};

class TrayEmbed : public QX11EmbedWidget
{
    Q_OBJECT
public:
    TrayEmbed( bool kdeTray, QWidget* parent = NULL );
    bool kdeTray() const { return kde_tray; }

private:
    bool kde_tray;
};

#endif
