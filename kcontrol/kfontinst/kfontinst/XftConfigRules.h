#ifndef __XFT_CONFIG_RULES_H__
#define __XFT_CONFIG_RULES_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigRules
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 14/06/2001
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

#include "XftConfigRulesData.h"
#include <qpushbutton.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_XFT
#include "XftConfigIncludesWidget.h"
#include "XftConfig.h"

class CXftConfigEditor;
#endif

class CXftConfigRules : public CXftConfigRulesData
{
    Q_OBJECT

    public:

    CXftConfigRules(QWidget *parent, const char *name=NULL);
    virtual ~CXftConfigRules() {}

#ifdef HAVE_XFT
    bool display();
    void itemSelected(QListViewItem *);
    void addButtonPressed();
    void editButtonPressed();
    void removeButtonPressed();
    void display(CXftConfig::TEntry *rule);

    QList<CXftConfig::TEntry> & getList()       { return itsRulesList; }
    QStringList                 getIncludes()   { return itsIncludes->getList(); }
    QStringList                 getIncludeIfs() { return itsIncludeIfs->getList(); }

#endif
    public slots:

    void enableOk() { itsOkButton->setEnabled(true); }

#ifdef HAVE_XFT
    private:

    QList<CXftConfig::TEntry> itsRulesList;
    CXftConfigEditor          *itsEditor;
#endif
};

#endif 
