/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2002 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KHLISTBOX_H_
#define _KHLISTBOX_H_

#include <qtimer.h>

#include <klistbox.h>

namespace KHotKeys
{

class KHListBox
    : public Q3ListBox
    {
    Q_OBJECT
    Q_PROPERTY( bool forceSelect READ forceSelect WRITE setForceSelect )
    public:
        KHListBox( QWidget* parent_P, const char* name_P = NULL );
        virtual void clear();
        virtual void insertItem( Q3ListBoxItem* item_P );
        bool forceSelect() const;
        void setForceSelect( bool force_P );
    Q_SIGNALS:
        void current_changed( Q3ListBoxItem* item_P );
    private Q_SLOTS:
        void slot_selection_changed( Q3ListBoxItem* item_P );
        void slot_selection_changed();
        void slot_current_changed( Q3ListBoxItem* item_P );
        void slot_insert_select();
    private:
        Q3ListBoxItem* saved_current_item;
        bool in_clear;
        bool force_select;
        QTimer insert_select_timer;
    };

//***************************************************************************
// Inline
//***************************************************************************

inline
void KHListBox::setForceSelect( bool force_P )
    {
    force_select = force_P;
    }
    
inline
bool KHListBox::forceSelect() const
    {
    return force_select;
    }

} // namespace KHotKeys

#endif
