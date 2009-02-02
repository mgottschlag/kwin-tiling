#ifndef WINDOW_DEFINITION_H
#define WINDOW_DEFINITION_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "hotkeys_widget_iface.h"

#include "qwindowdefs.h"
#include <QtGui/QWidget>
#include <KDE/KDialog>


namespace Ui {
    class WindowDefinitionWidget;
}

namespace KHotKeys {
    class Windowdef_simple;
}


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class WindowDefinitionWidget : public HotkeysWidgetIFace
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    WindowDefinitionWidget(KHotKeys::Windowdef_simple *windowdef, QWidget *parent = NULL);

    /**
     * Destructor
     */
    virtual ~WindowDefinitionWidget();

    bool isChanged() const;

protected:

    void doCopyFromObject();
    void doCopyToObject();

private Q_SLOTS:

    void slotWindowClassChanged(int);
    void slotWindowRoleChanged(int);
    void slotWindowTitleChanged(int);

    void slotAutoDetect();
    void slotWindowSelected(WId);

private:

    Ui::WindowDefinitionWidget *ui;

    KHotKeys::Windowdef_simple *_windowdef;
};


class WindowDefinitionDialog : public KDialog
    {
    Q_OBJECT

public:

    WindowDefinitionDialog( KHotKeys::Windowdef_simple *windowdef, QWidget *parent=NULL)
        :   KDialog(parent)
            ,def(NULL)
        {
        def = new WindowDefinitionWidget(windowdef, this);
        setMainWidget(def);
        def->copyFromObject();
        }


    ~WindowDefinitionDialog()
        {
        def = NULL;
        }


    virtual void accept()
        {
        def->copyToObject();
        KDialog::accept();
        }

private:

    WindowDefinitionWidget *def;
    };


#endif /* #ifndef WINDOW_DEFINITION_H */
