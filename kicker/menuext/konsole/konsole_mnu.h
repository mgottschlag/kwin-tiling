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

#ifndef _konsole_mnu_h_
#define _konsole_mnu_h_

#include <QVector>

#include <kpanelmenu.h>
#include <klibloader.h>

#include "konsolebookmarkmenu.h"
#include "konsolebookmarkhandler.h"


class KonsoleMenu : public KPanelMenu/*, public KPReloadObject*/
{
    Q_OBJECT

public:
    KonsoleMenu(QWidget *parent = 0);
    ~KonsoleMenu();
    KUrl baseURL() const;


protected Q_SLOTS:
    void slotExec(int id);
    void launchProfile(int id);
    void initialize();
    void newSession(const QString& sURL, const QString& title);
    //void makeGUI();


private:
    QStringList sessionList;
    QStringList screenList;
    QVector<QString> m_profiles;
    KMenu* m_profileMenu;
    KMenu* m_bookmarksSession;

    KonsoleBookmarkHandler *m_bookmarkHandlerSession;
};

class KonsoleMenuFactory : public KLibFactory
{
public:
    KonsoleMenuFactory(QObject *parent = 0, const char *name = 0);

protected:
    QObject* createObject(QObject *parent = 0, const char *classname = "QObject", const QStringList& args = QStringList());
};

#endif
