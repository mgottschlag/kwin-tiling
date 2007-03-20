/*  Copyright 2004, Daniel Woods Bullok <dan.devel@bullok.com>
    distributed under the terms of the
    GNU GENERAL PUBLIC LICENSE Version 2 -
    See the file kdebase/COPYING for details
*/

#ifndef __quickbuttongroup_h__
#define __quickbuttongroup_h__

#include <QString>
#include <functional>
#include "easyvector.h"
#include "quickbutton.h"


class QuickButtonGroup: virtual public EasyVector< QuickButton* > {
public:
    QuickButtonGroup(const EasyVector< QuickButton* > &kv):EasyVector< QuickButton* >(kv){}
    QuickButtonGroup():EasyVector< QuickButton* >(){}
    Index findDescriptor(const QString &desc);

    void show();
    void hide();
    void setDragging(bool drag);
    void setEnableDrag(bool enable);
    void deleteContents();
    void setUpdatesEnabled(bool enable);
};

QuickButtonGroup::Index QuickButtonGroup::findDescriptor(const QString &desc)
{   return findProperty(desc, std::mem_fun(&QuickButton::url));}

inline void QuickButtonGroup::setUpdatesEnabled(bool enable)
{   for (QuickButtonGroup::iterator i=begin();i!=end();++i) {
        (*i)->setUpdatesEnabled(enable);
        if (enable) { (*i)->update();}
    }
}

inline void QuickButtonGroup::show()
{   std::for_each(begin(),end(),std::mem_fun(&QWidget::show));}

inline void QuickButtonGroup::hide()
{   std::for_each(begin(),end(),std::mem_fun(&QWidget::hide));}

inline void QuickButtonGroup::setDragging(bool drag)
{   std::for_each(begin(),end(),std::bind2nd(std::mem_fun(&QuickButton::setDragging),drag));}

inline void QuickButtonGroup::setEnableDrag(bool enable)
{   std::for_each(begin(),end(),std::bind2nd(std::mem_fun(&QuickButton::setEnableDrag),enable));}

inline void QuickButtonGroup::deleteContents()
{   for (QuickButtonGroup::iterator i=begin();i!=end();++i) {
        delete (*i);
        (*i)=0;
    }
}

#endif

