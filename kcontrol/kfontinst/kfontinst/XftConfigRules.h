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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_XFT

#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif
#include <qpushbutton.h>
#include "XftConfigRulesData.h"
#include "XftConfigIncludesWidget.h"
#include "XftConfig.h"

class CXftConfigEditor;

class CXftConfigRules : public CXftConfigRulesData
{
    Q_OBJECT

    public:

    CXftConfigRules(QWidget *parent, const char *name=NULL);
    virtual ~CXftConfigRules() {}

    bool display();
    void itemSelected(QListViewItem *);
    void addButtonPressed();
    void editButtonPressed();
    void removeButtonPressed();
    void display(CXftConfig::TEntry *rule);
#if QT_VERSION >= 300
    QPtrList<CXftConfig::TEntry> & getList()    { return itsRulesList; }
#else
    QList<CXftConfig::TEntry> & getList()       { return itsRulesList; }
#endif
    QStringList                 getIncludes()   { return itsIncludes->getList(); }
    QStringList                 getIncludeIfs() { return itsIncludeIfs->getList(); }

    public slots:

    void enableOk() { itsOkButton->setEnabled(true); }

    private:

#if QT_VERSION >= 300
    QPtrList<CXftConfig::TEntry> itsRulesList;
#else
    QList<CXftConfig::TEntry> itsRulesList;
#endif
    CXftConfigEditor          *itsEditor;
};

#endif

#endif 
