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

#include "XftConfigRules.h"

#ifdef HAVE_XFT
#include <qlistview.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qdialog.h>
#include <qregexp.h>
#include "Misc.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "XftConfigEditor.h"

class CXftConfigListViewItem : public QListViewItem
{
    public:
 
    CXftConfigListViewItem(CXftConfig::TEntry *rule, QListView *view, const QString &test, const QString &edit)
        : QListViewItem(view, test, edit), itsRule(rule) {}
 
    virtual ~CXftConfigListViewItem() {}

    CXftConfig::TEntry * rule()       { return itsRule; }
 
    private:
 
    CXftConfig::TEntry *itsRule;
};

#endif

CXftConfigRules::CXftConfigRules(QWidget *parent, const char *name)
               : CXftConfigRulesData(parent, name, true)
#ifdef HAVE_XFT
       , itsEditor(NULL)
#endif
{
#ifdef HAVE_XFT
    itsIncludes->setName(i18n("Include:"));
    itsIncludeIfs->setName(i18n("Include If:"));
    connect(itsIncludes, SIGNAL(changed()), SLOT(enableOk()));
    connect(itsIncludeIfs, SIGNAL(changed()), SLOT(enableOk()));
#endif
}

#ifdef HAVE_XFT
bool CXftConfigRules::display()
{
    CXftConfig::TEntry *entry;

    itsList->clear();
    itsIncludes->setList(CKfiGlobal::xft().getIncludes());
    itsIncludeIfs->setList(CKfiGlobal::xft().getIncludeIfs());
    itsRulesList=CKfiGlobal::xft().getEntries();

    itsOkButton->setEnabled(false);
    itsAddButton->setEnabled(CMisc::fWritable(CKfiGlobal::cfg().getXftConfigFile()));

    for(entry=itsRulesList.first(); entry; entry=itsRulesList.next())
        display(entry);

    itsRemoveButton->setEnabled(false);
    itsEditButton->setEnabled(false);
    itsAddButton->setEnabled(CMisc::fWritable(CKfiGlobal::cfg().getXftConfigFile()));

    if(QDialog::Accepted!=exec())  // Remove any newly created items...
    {
        for(entry=itsRulesList.first(); entry; entry=itsRulesList.next())
            if(-1==CKfiGlobal::xft().getEntries().findRef(entry))
                delete entry;
        return false;
    }
    else
        return true;
}

void CXftConfigRules::itemSelected(QListViewItem *item)
{
    bool enable=false;

    if(NULL!=item)
        enable=CMisc::fWritable(CKfiGlobal::cfg().getXftConfigFile());

    itsRemoveButton->setEnabled(enable);
    itsEditButton->setEnabled(enable);
}

void CXftConfigRules::addButtonPressed()
{
    if(NULL==itsEditor)
        itsEditor=new CXftConfigEditor(this);

    CXftConfig::TEntry *res=itsEditor->display(NULL);

    if(NULL!=res)
    {
        itsRulesList.append(res);
        display(res);
        enableOk();
    }
}

void CXftConfigRules::editButtonPressed()
{
    CXftConfigListViewItem *item=(CXftConfigListViewItem *)itsList->currentItem();
 
    if(NULL==item)
        KMessageBox::information(this, i18n("No item selected!"), i18n("Oops..."));
    else
    {
        if(NULL==itsEditor)
            itsEditor=new CXftConfigEditor(this);
 
        CXftConfig::TEntry *res=itsEditor->display(item->rule());

        if(NULL!=res)
        {
            itsRulesList.remove(item->rule());
            delete item;
            itsRulesList.append(res);
            display(res);
            enableOk();
        }
    }
}

void CXftConfigRules::removeButtonPressed()
{
    CXftConfigListViewItem *item=(CXftConfigListViewItem *)itsList->currentItem();

    if(NULL==item)
        KMessageBox::information(this, i18n("No item selected!"), i18n("Oops..."));
    else
        if(KMessageBox::questionYesNo(this, i18n("Remove selected rule"), i18n("Remove?"))==KMessageBox::Yes)
        {
            itsRulesList.remove(item->rule());
            delete item;
            enableOk();
            itsRemoveButton->setEnabled(false);
            itsEditButton->setEnabled(false);
        }
}

void CXftConfigRules::display(CXftConfig::TEntry *rule)
{
    QCString test(rule->testStr()),
             edit(rule->editStr());

    test.replace(QRegExp(" \\n"), ", and");
    test=test.simplifyWhiteSpace();
    edit=edit.simplifyWhiteSpace();

    new CXftConfigListViewItem(rule, itsList, test, edit);
}

#endif
#include "XftConfigRules.moc"
