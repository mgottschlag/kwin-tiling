#ifndef WINDOW_DEFINITION_LIST_WIDGET_H
#define WINDOW_DEFINITION_LIST_WIDGET_H
/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

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

#include "ui_window_definition_list_widget.h"
#include "windows_helper/window_selection_list.h"



/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class WindowDefinitionListWidget : public HotkeysWidgetIFace
    {
    Q_OBJECT

public:

    /**
     * Default constructor
     */
    WindowDefinitionListWidget(
            KHotKeys::Windowdef_list *windowdef_list,
            QWidget *parent = NULL);

    WindowDefinitionListWidget(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~WindowDefinitionListWidget();

    void setWindowDefinitions(KHotKeys::Windowdef_list *windowdef_list);

    bool isChanged() const;

private:

    void emitChanged(bool);

private Q_SLOTS:

    void slotDelete(bool);
    void slotDuplicate(bool);
    void slotEdit(bool);
    void slotNew(bool);

protected:

    void doCopyFromObject();
    void doCopyToObject();

private:

    // The Windowdefinition list
    KHotKeys::Windowdef_list    *_windowdefs;
    KHotKeys::Windowdef_list    *_working;

    // The User Interface
    Ui::WindowDefinitionListWidget ui;

    // Unsaved changes?
    bool _changed;
};


class WindowDefinitionListDialog : public KDialog
    {
    Q_OBJECT

public:

    WindowDefinitionListDialog(
            KHotKeys::Windowdef_list *list,
            QWidget *parent=NULL)
        :   KDialog(parent)
            ,def(NULL)
        {
        def = new WindowDefinitionListWidget(list, this);
        setMainWidget(def);
        def->copyFromObject();
        }


    ~WindowDefinitionListDialog()
        {
        def = NULL;
        }


    virtual void accept()
        {
        def->copyToObject();
        KDialog::accept();
        }

private:

    WindowDefinitionListWidget *def;
    };

#endif /* #ifndef WINDOW_DEFINITION_LIST_WIDGET_H */
