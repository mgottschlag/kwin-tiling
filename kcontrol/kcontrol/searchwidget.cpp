/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2004 Daniel Molkentin <molkentin@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <QLabel>
#include <QLayout>
#include <QRegExp>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3PtrList>

#include <klineedit.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klistbox.h>

#include "searchwidget.h"
#include "searchwidget.moc"

/**
 * Helper class for sorting icon modules by name without losing the fileName ID
 */
class ModuleItem : public Q3ListBoxPixmap
{
public:
 ModuleItem(ConfigModule *module, Q3ListBox * listbox = 0) :
	Q3ListBoxPixmap(listbox,
      KGlobal::iconLoader()->loadIcon(module->icon(), K3Icon::Desktop, K3Icon::SizeSmall),
      module->moduleName())
  , m_module(module)
 { 
 
 }

 ConfigModule *module() const { return m_module; };

protected:
 ConfigModule *m_module;

};

KeywordListEntry::KeywordListEntry(const QString& name, ConfigModule* module)
  : _name(name)
{
  if(module)
    _modules.append(module);
}

void KeywordListEntry::addModule(ConfigModule* module)
{
  if(module)
    _modules.append(module);
}

SearchWidget::SearchWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  _keywords.setAutoDelete(true);

  QVBoxLayout * l = new QVBoxLayout(this);
  l->setMargin(0);
  l->setSpacing(2);

  // input
  _input = new KLineEdit(this);
  _input->setFocus();
  QLabel *inputl = new QLabel(i18n("Se&arch:"), this);
  inputl->setBuddy(_input);

  l->addWidget(inputl);
  l->addWidget(_input);

  // keyword list
  _keyList = new KListBox(this);
  QLabel *keyl = new QLabel(i18n("&Keywords:"), this);
  keyl->setBuddy(_keyList);

  l->addWidget(keyl);
  l->addWidget(_keyList);

  // result list
  _resultList = new KListBox(this);
  QLabel *resultl = new QLabel(i18n("&Results:"), this);
  resultl->setBuddy(_resultList);

  l->addWidget(resultl);
  l->addWidget(_resultList);

  // set stretch factors
  l->setStretchFactor(_resultList, 1);
  l->setStretchFactor(_keyList, 2);


  connect(_input, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotSearchTextChanged(const QString&)));

  connect(_keyList, SIGNAL(highlighted(const QString&)),
          this, SLOT(slotKeywordSelected(const QString&)));

  connect(_resultList, SIGNAL(selected(Q3ListBoxItem*)),
          this, SLOT(slotModuleSelected(Q3ListBoxItem *)));
  connect(_resultList, SIGNAL(clicked(Q3ListBoxItem *)),
          this, SLOT(slotModuleClicked(Q3ListBoxItem *)));
}

void SearchWidget::populateKeywordList(ConfigModuleList *list)
{
  ConfigModule *module;

  // loop through all control modules
  for (module=list->first(); module != 0; module=list->next())
    {
      if (module->library().isEmpty())
        continue;

      // get the modules keyword list
      QStringList kw = module->keywords();

      // loop through the keyword list to populate _keywords
      for(QStringList::ConstIterator it = kw.begin(); it != kw.end(); ++it)
        {
          QString name = (*it).toLower();
          bool found = false;

          // look if _keywords already has an entry for this keyword
          for(KeywordListEntry *k = _keywords.first(); k != 0; k = _keywords.next())
            {
              // if there is an entry for this keyword, add the module to the entries modul list
              if (k->moduleName() == name)
                {
                  k->addModule(module);
                  found = true;
                  break;
                }
            }

          // if there is entry for this keyword, create a new one
          if (!found)
            {
              KeywordListEntry *k = new KeywordListEntry(name, module);
              _keywords.append(k);
            }
        }
    }
  populateKeyListBox("*");
}

void SearchWidget::populateKeyListBox(const QString& s)
{
  _keyList->clear();

  QStringList matches;

  for(KeywordListEntry *k = _keywords.first(); k != 0; k = _keywords.next())
    {
      if ( QRegExp(s, Qt::CaseInsensitive, QRegExp::Wildcard).indexIn(k->moduleName()) >= 0)
        matches.append(k->moduleName().trimmed());
    }

  for(QStringList::ConstIterator it = matches.begin(); it != matches.end(); it++)
    _keyList->insertItem(*it);

  _keyList->sort();
}

void SearchWidget::populateResultListBox(const QString& s)
{
  _resultList->clear();

  Q3PtrList<ModuleItem> results;

  for(KeywordListEntry *k = _keywords.first(); k != 0; k = _keywords.next())
    {
      if (k->moduleName() == s)
        {
          Q3PtrList<ConfigModule> modules = k->modules();

          for(ConfigModule *m = modules.first(); m != 0; m = modules.next())
              new ModuleItem(m, _resultList);
        }
    }

  _resultList->sort();
}

void SearchWidget::slotSearchTextChanged(const QString & s)
{
  QString regexp = s;
  regexp += "*";
  populateKeyListBox(regexp);
}

void SearchWidget::slotKeywordSelected(const QString & s)
{
  populateResultListBox(s);
}

void SearchWidget::slotModuleSelected(Q3ListBoxItem *item)
{
  if (item)
    emit moduleSelected( static_cast<ModuleItem*>(item)->module() );
}

void SearchWidget::slotModuleClicked(Q3ListBoxItem *item)
{
  if (item)
    emit moduleSelected( static_cast<ModuleItem*>(item)->module() );
}
