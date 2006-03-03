/*****************************************************************

Copyright (c) 2005 Marc Cramdal
Copyright (c) 2005 Aaron Seigo <aseigo@kde.org>

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

#ifndef __addapplet_h__
#define __addapplet_h__

#include <qstringlist.h>
#include <qpixmap.h>

#include <QList>
#include <QCloseEvent>

#include <klocale.h>
#include <kdialog.h>

#include "appletinfo.h"

class ContainerArea;
class AppletView;
class AppletWidget;

class AddAppletDialog : public KDialog
{
    Q_OBJECT

    public:
        AddAppletDialog(ContainerArea* cArea, QWidget* parent, const char* name);
        void updateInsertionPoint();

    protected:
        void closeEvent(QCloseEvent*);

    private Q_SLOTS:
        void populateApplets();
        void addCurrentApplet();
        void addApplet(AppletWidget* applet);
        void search(const QString &s);
        void filter(int i);
        void selectApplet(AppletWidget* applet);

    private:
        bool appletMatchesSearch(const AppletWidget* w, const QString& s);

        AppletView *m_mainWidget;

        AppletInfo::List m_applets;

        QList<AppletWidget*> m_appletWidgetList;
        AppletWidget* m_selectedApplet;

        ContainerArea* m_containerArea;
        AppletInfo::AppletType m_selectedType;
        QPoint m_insertionPoint;
        bool m_closing;
};

#endif
