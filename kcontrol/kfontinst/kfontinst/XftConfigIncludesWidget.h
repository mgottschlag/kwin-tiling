#ifndef __XFT_CONFIG_INCLUDES_WIDGET_H__
#define __XFT_CONFIG_INCLUDES_WIDGET_H__
 
////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigIncludesWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 04/07/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "XftConfigIncludesWidgetData.h"
#include <qstringlist.h>
#include <qgroupbox.h>

class CXftConfigIncludesWidget : public CXftConfigIncludesWidgetData
{
    Q_OBJECT

    public:

    CXftConfigIncludesWidget(QWidget *parent=NULL, const char *name=NULL) : CXftConfigIncludesWidgetData(parent, name) {}
    virtual ~CXftConfigIncludesWidget()                                                                                {}

#ifdef HAVE_XFT
    void setName(const QString &name) { itsGroupBox->setTitle(name); itsWritable=false; }

    void setList(const QStringList &files);
    void addPressed();
    void editPressed();
    void removePressed();
    void itemSelected(QListBoxItem *item);

    QStringList getList();
#endif

    signals:

    void changed();

#ifdef HAVE_XFT
    private:

    QString getFile(const QString &current, bool checkDuplicates=false);
    bool    itsWritable;
#endif
};

#endif
