/*****************************************************************

Copyright (c) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
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

#include <QStringList>
#include <QPixmap>

#include <QList>
#include <QCloseEvent>

#include <klocale.h>
#include <kdialog.h>

#include "appletinfo.h"
#include "ui_appletview.h"
#include "appletlistmodel.h"
#include "appletitemdelegate.h"

class ContainerArea;

class AddAppletDialog : public KDialog
{
    Q_OBJECT

public:
    AddAppletDialog(ContainerArea *cArea, QWidget *parent, const char *name);
    void updateInsertionPoint();

protected:
    void closeEvent(QCloseEvent *);

private Q_SLOTS:
    void populateApplets();
    void selectApplet(const QModelIndex &applet);
    void addCurrentApplet(const QModelIndex &selectedApplet);
    void search(const QString &s);
    void filter(int i);
    void slotUser1Clicked();
    void updateAppletList();

private:
    bool appletMatchesSearch(const AppletInfo *i, const QString &s);

    AppletListModel *m_listModel;
    Ui::AppletView *m_mainWidgetView;
    QWidget *m_mainWidget;

    QModelIndex selectedApplet;
    AppletInfo::List m_applets;
    ContainerArea *m_containerArea;
    AppletInfo::AppletType m_selectedType;
    QPoint m_insertionPoint;
};

#endif
