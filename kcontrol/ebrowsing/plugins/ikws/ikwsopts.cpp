/*
 * Copyright (c) 2000 Yves Arrouye <yves@realnames.com>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <iostream>
#include <unistd.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <kbuttonbox.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdialog.h>

#include "ikwsopts.h"

#define ITEM_NONE   (i18n("None"))
#define searcher    KURISearchFilterEngine::self()

InternetKeywordsOptions::InternetKeywordsOptions(QWidget *parent, const char *name)
                        :KCModule(parent, name)
{
    // This is the layout for the whole dialog box.
    QVBoxLayout *lay = new QVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

    cb_enableInternetKeywords = new QCheckBox(i18n("Enable Int&ernet Keywords"), this);
    cb_enableInternetKeywords->adjustSize();
    cb_enableInternetKeywords->setMinimumSize(cb_enableInternetKeywords->size());
    connect(cb_enableInternetKeywords, SIGNAL(clicked()), this, SLOT(changeInternetKeywordsEnabled()));
    QWhatsThis::add(cb_enableInternetKeywords, i18n("If this box is checked, KDE will let you type Internet Keywords in its browser's address bar."));

/* Should replace the above after KDE 2.0...
    QWhatsThis::add(cb_enableInternetKeywords, i18n("If this box is checked, KDE will let you use <em>Internet Keywords</em> "
                                                    "in its browser's address bar. This means you can simply type normal "
                                                    "words and phrases, such as \"KDE\", to automatically get "
                                                    "re-directed to the appropriate site. For further details on this "
                                                    "feature visit <em>www.internetkeywords.org</em>."));
*/
    lay->addWidget( cb_enableInternetKeywords );

    // We add the Fallback config stuff to a horizontal layout manager
    // first so that we can: 1) Add the label and the combo side by side
    // and two give it an indented look so that the option looks under
    // the Enable IKWS checkbox.
    QHBoxLayout *igbopts_lay = new QHBoxLayout( lay, KDialog::spacingHint() );

    lb_searchFallback = new QLabel(i18n("Search &Fallback:"),this);
    igbopts_lay->addWidget( lb_searchFallback );

    cmb_searchFallback = new QComboBox(false, this);
    igbopts_lay->addWidget( cmb_searchFallback );
    lb_searchFallback->setBuddy(cmb_searchFallback);

    // Pad the beginning and the end with space to give it an indented look.
    igbopts_lay->insertSpacing( 0, 4 * KDialog::spacingHint() );
    igbopts_lay->insertSpacing( -1, 10 * KDialog::spacingHint() );

    connect(cmb_searchFallback, SIGNAL(activated(const QString &)), this, SLOT(changeSearchFallback(const QString &)));
    QString wtstr = i18n("Here you can select a search engine that will be used if what is typed is not an Internet Keyword. Select None if you do not want to do a search in this case.");

/* Should replace the above after KDE 2.0...
    QString wtstr = i18n("Allows you to select a search engine that will be used in case what you "
                         "typed is not an <em>Internet Keyword</em>.  Select \"None\" if you do not "
                         "want to do a search in this case.");
*/
    QWhatsThis::add(lb_searchFallback, wtstr);
    QWhatsThis::add(cmb_searchFallback, wtstr);

/*  Feature to consider after KDE 2.0...
    cb_enableSearchKeywords = new QCheckBox(i18n("Enable &Search Engine Shortcuts"), this);
    cb_enableSearchKeywords->adjustSize();
    cb_enableSearchKeywords->setMinimumSize(cb_enableSearchKeywords->size());
    connect(cb_enableSearchKeywords, SIGNAL(clicked()), this, SLOT(changeSearchKeywordsEnabled()));
    QWhatsThis::add(cb_enableSearchKeywords, i18n("If this box is checked, KDE will let you use the shortcuts "
                                                  "defined below to quickly search the internet. For example, "
                                                  "typing the words <em>short-cut</em>:<b>KDE</b> will result in "
                                                  "the word <em>KDE</em> being searched using the URL defined by "
                                                  "the <em>short-cut</em>."));
    lay->addWidget( cb_enableSearchKeywords );
*/
    gb_search = new QGroupBox( i18n("Search"), this );
    QGridLayout *gbLay = new QGridLayout(gb_search, 11, 3, KDialog::marginHint(), KDialog::spacingHint());

    gbLay->addRowSpacing(0, 2* KDialog::spacingHint());
    gbLay->addRowSpacing(3, 2* KDialog::spacingHint());
    gbLay->addRowSpacing(4, 2* KDialog::spacingHint());
    gbLay->addRowSpacing(6, 2* KDialog::spacingHint());
    gbLay->addColSpacing(1, 2* KDialog::spacingHint());
    gbLay->setRowStretch(9, 1);
    gbLay->setColStretch(0, 3);
    gbLay->setColStretch(2, 4); 

    lv_searchProviders = new QListView( gb_search );
    lv_searchProviders->setMultiSelection(false);
    lv_searchProviders->addColumn(i18n("Name"));
    lv_searchProviders->addColumn(i18n("Shortcuts"));
    lv_searchProviders->setSorting(0);
    lv_searchProviders->setMinimumSize(lv_searchProviders->sizeHint());
    wtstr = i18n("This list contains the search engines that KDE knows about, and their associated shortcuts.");
    QWhatsThis::add(lv_searchProviders, wtstr);

    connect(lv_searchProviders, SIGNAL(selectionChanged(QListViewItem *)),
           this, SLOT(updateSearchProvider(QListViewItem *)));

    gbLay->addMultiCellWidget(lv_searchProviders, 1, 9, 0, 0);

    lb_searchProviderName = new QLabel(i18n("Search &Provider Name:"), gb_search);
    lb_searchProviderName->adjustSize();
    lb_searchProviderName->setMinimumSize(lb_searchProviderName->size());    
    gbLay->addWidget(lb_searchProviderName, 1, 2);

    le_searchProviderName = new QLineEdit(gb_search);
    le_searchProviderName->adjustSize();
    le_searchProviderName->setMinimumSize(le_searchProviderName->size()); 
    gbLay->addWidget(le_searchProviderName, 2, 2);

    lb_searchProviderName->setBuddy(le_searchProviderName);
    connect(le_searchProviderName, SIGNAL(textChanged(const QString &)),
         SLOT(textChanged(const QString &)));
    wtstr = i18n("Enter the human readable name of the search engine here.");
    QWhatsThis::add(lb_searchProviderName, wtstr);
    QWhatsThis::add(le_searchProviderName, wtstr);

    lb_searchProviderURI = new QLabel(i18n("Search &URI:"), gb_search);
    lb_searchProviderURI->adjustSize();
    lb_searchProviderURI->setMinimumSize(lb_searchProviderURI->size());
    gbLay->addWidget(lb_searchProviderURI, 3, 2); 
    
    le_searchProviderURI = new QLineEdit(gb_search);
    le_searchProviderURI->adjustSize();
    le_searchProviderURI->setMinimumSize(le_searchProviderURI->size());  
    gbLay->addWidget(le_searchProviderURI, 4, 2);

    lb_searchProviderURI->setBuddy(le_searchProviderURI);
    connect(le_searchProviderURI, SIGNAL(textChanged(const QString &)),
         SLOT(textChanged(const QString &)));
    wtstr = i18n("Enter the URI that is used to do a search on the search engine here. The text to be searched for can be specified as \\1.");
    QWhatsThis::add(lb_searchProviderURI, wtstr);
    QWhatsThis::add(le_searchProviderURI, wtstr);

    lb_searchProviderShortcuts = new QLabel(i18n("UR&I Shortcuts:"), gb_search);
    lb_searchProviderShortcuts->adjustSize();
    lb_searchProviderShortcuts->setMinimumSize(lb_searchProviderShortcuts->size());
    gbLay->addWidget(lb_searchProviderShortcuts, 5, 2);

    le_searchProviderShortcuts = new QLineEdit(gb_search);
    le_searchProviderShortcuts->adjustSize();
    le_searchProviderShortcuts->setMinimumSize(le_searchProviderShortcuts->size());
    gbLay->addWidget(le_searchProviderShortcuts, 6, 2);

    lb_searchProviderShortcuts->setBuddy(le_searchProviderShortcuts);
    connect(le_searchProviderShortcuts, SIGNAL(textChanged(const QString &)),
         SLOT(textChanged(const QString &)));
    wtstr = i18n("The shortcuts entered here can be used as a pseudo-URI scheme in KDE. For example, the shortcut av can be used as in av:my search.");
    QWhatsThis::add(lb_searchProviderShortcuts, wtstr);
    QWhatsThis::add(le_searchProviderShortcuts, wtstr);

    KButtonBox *bbox = new KButtonBox( gb_search );
    bbox->addStretch(20);
    pb_chgSearchProvider = bbox->addButton(i18n("&Add"));
    QWhatsThis::add(pb_chgSearchProvider, i18n("Click here to add/change a search engine."));
    connect(pb_chgSearchProvider, SIGNAL(clicked()), this, SLOT(changeSearchProvider()));
    pb_delSearchProvider = bbox->addButton(i18n("&Delete"));
    QWhatsThis::add(pb_delSearchProvider, i18n("Click here to delete the currently selected search engine from the list."));
    pb_delSearchProvider->setEnabled(false);
    connect(pb_delSearchProvider, SIGNAL(clicked()), this, SLOT(deleteSearchProvider()));
    gbLay->addWidget(bbox, 7, 2);
    bbox->layout();
    lay->addWidget( gb_search );

    // Load the options
    load();
}

QString InternetKeywordsOptions::quickHelp() const
{
    return i18n("In this module, you can configure various Internet search engines "
                "that KDE can use for keyword search. This allows you, for example, "
                "to enter a pseudo-URI like gg:Smetana to search the Google search "
                "engine for web pages about the Czech composer Bedrich Smetana.");
}

void InternetKeywordsOptions::load()
{
    // Clear state first.
    lv_searchProviders->clear();
    cmb_searchFallback->clear();
    cmb_searchFallback->insertItem(ITEM_NONE);

    le_searchProviderName->clear();
    le_searchProviderShortcuts->clear();
    le_searchProviderURI->clear();

    // Go!
    searcher->loadConfig();

    QString searchFallbackName = searcher->searchFallback();
    QValueList<KURISearchFilterEngine::SearchEntry> lstSearchEngines = searcher->searchEngines();
    QValueList<KURISearchFilterEngine::SearchEntry>::ConstIterator it = lstSearchEngines.begin();
    QValueList<KURISearchFilterEngine::SearchEntry>::ConstIterator end = lstSearchEngines.end();

    for (; it != end; ++it)
      displaySearchProvider(*it, searchFallbackName == (*it).m_strName);\

    // Enable/Disable widgets accordingly.
    bool ikwsEnabled = searcher->isInternetKeywordsEnabled();
    cb_enableInternetKeywords->setChecked( ikwsEnabled );
    cmb_searchFallback->setEnabled( ikwsEnabled );
    lb_searchFallback->setEnabled( ikwsEnabled );

    // Feature to be considered after KDE 2.0...
    // bool searchEnabled = searcher->isSearchKeywordsEnabled();
    // cb_enableSearchKeywords->setChecked( searchEnabled );
    // gb_search->setEnabled( searchEnabled );

    if (lv_searchProviders->childCount())
      lv_searchProviders->setSelected(lv_searchProviders->firstChild(), true);
}

void InternetKeywordsOptions::save()
{
    searcher->saveConfig();
    QByteArray data;
    kapp->dcopClient()->send("*", "KURIIKWSFilterIface", "configure()", data);
    kapp->dcopClient()->send("*", "KURISearchFilterIface", "configure()", data);
}

void InternetKeywordsOptions::defaults()
{
    load();
}

void InternetKeywordsOptions::moduleChanged(bool state)
{
    emit changed(state);
}

void InternetKeywordsOptions::changeInternetKeywordsEnabled()
{
    bool use_keywords = cb_enableInternetKeywords->isChecked();
    cmb_searchFallback->setEnabled(use_keywords);
    lb_searchFallback->setEnabled(use_keywords);
    searcher->setInternetKeywordsEnabled(use_keywords);
    moduleChanged(true);
}

void InternetKeywordsOptions::changeSearchKeywordsEnabled()
{
/* Feature to consider for after KDE 2.0...
    bool use_keywords = cb_enableSearchKeywords->isChecked();
    gb_search->setEnabled(use_keywords);
    searcher->setSearchKeywordsEnabled(use_keywords);
    moduleChanged(true);
*/
}

void InternetKeywordsOptions::changeSearchFallback(const QString &name)
{
    searcher->setSearchFallback(name == ITEM_NONE ? QString::null : name);
    moduleChanged(true);
}

void InternetKeywordsOptions::textChanged(const QString &)
{
    QString provider = le_searchProviderName->text();
    QString uri = le_searchProviderURI->text();
    QString shortcuts = le_searchProviderShortcuts->text();
    bool known = false;
    bool same = false;

    KURISearchFilterEngine::SearchEntry e = searcher->searchEntryByName(provider);
    if (known = (!e.m_strName.isNull()))
    {
      pb_chgSearchProvider->setText(i18n("Ch&ange"));
      same = e.m_strQuery == uri && e.m_lstKeys.join(", ") == shortcuts;
    }
    else
    {
      pb_chgSearchProvider->setText(i18n("&Add"));
    }

    if (!same && !provider.isEmpty() && !uri.isEmpty())
    {
      pb_chgSearchProvider->setEnabled(true);
    }
    else
    {
      pb_chgSearchProvider->setEnabled(false);
    }

    pb_delSearchProvider->setEnabled(known);
}

void InternetKeywordsOptions::changeSearchProvider()
{
    QString provider = le_searchProviderName->text();
    QString uri = le_searchProviderURI->text();

    if( provider.isEmpty() )
    {
        KMessageBox::error( 0, i18n("You must enter a search provider name first!") );
        return;
    }
    if( uri.isEmpty() )
    {
        KMessageBox::error( 0, i18n("You must enter a search provider URI first!") );
        return;
    }

    if (!uri.isEmpty())
    {
      QString tmp(uri);
      if (tmp.find("\\1") < 0)
      {
        if (KMessageBox::warningContinueCancel(0,
            i18n("The URI does not contain a \\1 placeholder for the user query.\nThis means that the same page is always going to be visited, \nregardless of what the user types..."), QString::null, i18n("Keep It")) == KMessageBox::Cancel)
        {
            return;
        }
      }
    }

    KURISearchFilterEngine::SearchEntry entry;

    entry.m_strName = provider;
    entry.m_strQuery = uri;
    entry.m_lstKeys = QStringList::split(", ", le_searchProviderShortcuts->text());

    searcher->insertSearchEngine(entry);
    lv_searchProviders->setSelected(displaySearchProvider(entry), true);

    pb_chgSearchProvider->setEnabled(false);
    pb_delSearchProvider->setEnabled(true);

    moduleChanged(true);
}

void InternetKeywordsOptions::deleteSearchProvider()
{
    const QString &provider = le_searchProviderName->text();
    searcher->removeSearchEngine(provider);

    QListViewItemIterator lvit(lv_searchProviders), lvpit;
    for (; lvit.current(); ++lvit)
    {
      const QString &name = lvit.current()->text(0);
      if (name == provider)
      {
        lv_searchProviders->removeItem(lvit.current());
        lv_searchProviders->setSelected(lvit.current(), true);
        break;
      }
    }

    // Update the combo box to go to None if the fallback was deleted.
    int current = cmb_searchFallback->currentItem();
    for (int i = 1, count = cmb_searchFallback->count(); i < count; ++i)
    {
      if (cmb_searchFallback->text(i) == provider)
      {
        cmb_searchFallback->removeItem(i);
        if (current >= i)
        {
          if (i == current)
            searcher->setSearchFallback(QString::null);
          cmb_searchFallback->setCurrentItem(current - 1);
        }
        cmb_searchFallback->update();
        break;
      }
    }

    pb_delSearchProvider->setEnabled(lv_searchProviders->childCount());
    moduleChanged(true);
}

void InternetKeywordsOptions::updateSearchProvider(QListViewItem *lvi)
{
    QString provider, uri, shortcuts;

    if (lvi)
    {
      const KURISearchFilterEngine::SearchEntry &e = searcher->searchEntryByName(lvi->text(0));
      provider = lvi->text(0);
      shortcuts = lvi->text(1);
      uri = e.m_strQuery;
    }

    le_searchProviderName->setText(provider);
    le_searchProviderURI->setText(uri);
    le_searchProviderShortcuts->setText(shortcuts);
    textChanged("");
    pb_delSearchProvider->setEnabled(lvi);
}

QListViewItem *InternetKeywordsOptions::displaySearchProvider(const KURISearchFilterEngine::SearchEntry &e, bool fallback) {

    // Show the provider in the list.
    QListViewItem *item = 0L;

    QListViewItemIterator it(lv_searchProviders);
    for (; it.current(); ++it)
    {
      if (it.current()->text(0) == e.m_strName)
      {
        item = it.current();
        break;
      }
    }

    if (!item)
    {
      item = new QListViewItem(lv_searchProviders);
      // Put the name in the combo box.
      int i, count = cmb_searchFallback->count();
      for (i = 1; i < count; ++i)
      {
        if (cmb_searchFallback->text(i) > e.m_strName)
        {
          int current = cmb_searchFallback->currentItem();
          cmb_searchFallback->insertItem(e.m_strName, i);
          if (current >= i)
            cmb_searchFallback->setCurrentItem(current + 1);
          break;
        }
      }
      if (i == count)
        cmb_searchFallback->insertItem(e.m_strName);

      if (fallback)
        cmb_searchFallback->setCurrentItem(i);
    }

    item->setText(0, e.m_strName);
    item->setText(1, e.m_lstKeys.join(", "));
    if (!it.current())
        lv_searchProviders->sort();

    return item;
}

#include "ikwsopts.moc"
