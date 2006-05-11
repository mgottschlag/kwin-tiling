/*****************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

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

#ifndef _prefmenu_h_
#define _prefmenu_h_

#include <QMap>
//Added by qt3to4:
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QList>

#include <kpanelmenu.h>
#include <ksycocaentry.h>

typedef QMap<int, KSycocaEntry::Ptr> EntryMap;

class PrefMenu : public KPanelMenu
{
    Q_OBJECT

    public:
        PrefMenu(QWidget *parent,
                 const QStringList & /*args*/);
        PrefMenu(const QString& label,
                 const QString& root,
                 QWidget *parent);
        ~PrefMenu();

    protected:
        void insertMenuItem(KService::Ptr & s,
                            int nId,
                            int nIndex= -1,
                            const QStringList *suppressGenericNames = 0);
        virtual void mousePressEvent(QMouseEvent *);
        virtual void mouseMoveEvent(QMouseEvent *);
        //virtual void dragEnterEvent(QDragEnterEvent *);
        //virtual void dragLeaveEvent(QDragLeaveEvent *);

        bool m_clearOnClose;
        QString m_root;
        QPoint m_dragStartPos;
        EntryMap m_entryMap;

    protected Q_SLOTS:
        void initialize();
        void slotExec(int id); // from KPanelMenu
        void slotClear(); // from KPanelMenu
        void clearOnClose();
        void aboutToClose();
        void launchControlCenter();
        void dragObjectDestroyed();
};

#endif
