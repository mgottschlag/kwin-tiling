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

#ifndef PANEL_APPLET_OP_MENU_H
#define PANEL_APPLET_OP_MENU_H

#include <QMenu>
//Added by qt3to4:
#include <QKeyEvent>

class AppletInfo;

// The button operations menu (usually right click)
class PanelAppletOpMenu : public QMenu
{
Q_OBJECT

public:
    enum OpButton{Move = 9900, Remove = 9901, Help = 9902, About = 9903, Preferences = 9904, ReportBug = 9905 };

    // Note: these entries have to be | and &-able with KPanelApplet::Actions!
    //       they also are treated just like Plasma::Preferences on selection
    //       KDE4: look at merging them there? perhaps under a generic "Editor" option?
    enum { KMenuEditor = 1048576, BookmarkEditor = 2097152 };
    PanelAppletOpMenu(int actions, QMenu *opMenu, const QMenu* appletsMenu = 0,
                      const QString &title = 0, const QString &icon = 0,
                      QWidget *parent=0);

Q_SIGNALS:
    void escapePressed();

protected:
    void keyPressEvent(QKeyEvent* e);
};


#endif // PANEL_APPLET_OP_MENU_H
