/*
 * Copyright (c) 2000 Yves Arrouye <yves@realnames.com>
 * Copyright (c) 2001, 2002 Dawit Alemayehu <adawit@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qfile.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kconfig.h>
#include <ktrader.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "ikwsopts.h"
#include "kuriikwsfiltereng.h"
#include "searchprovider.h"
#include "searchproviderdlg.h"


class SearchProviderItem : public QListViewItem
{
public:
    SearchProviderItem(QListView *parent, SearchProvider *provider)
    :QListViewItem(parent), m_provider(provider)
    {
      update();
    };

    virtual ~SearchProviderItem()
    {
      delete m_provider;
    }

    void update()
    {
      setText(0, m_provider->name());
      setText(1, m_provider->keys().join(","));
    }

    SearchProvider *provider() const { return m_provider; }

private:
    SearchProvider *m_provider;
};

FilterOptions::FilterOptions(KInstance *instance, QWidget *parent, const char *name)
              :KCModule(instance, parent, name)
{
    QVBoxLayout *lay = new QVBoxLayout( this, KDialog::marginHint(),
                                        KDialog::spacingHint() );
    lay->setAutoAdd( true );

    // Auto Web Search label and combobox
    QVBox *vbox = new QVBox (this);
    vbox->setSpacing (KDialog::spacingHint());
    lb_defaultSearchEngine = new QLabel(i18n("Defa&ult search engine:"), vbox);
    lb_defaultSearchEngine->setSizePolicy (QSizePolicy::Maximum,QSizePolicy::Preferred);

    cmb_defaultSearchEngine = new QComboBox(false, vbox);
    lb_defaultSearchEngine->setBuddy(cmb_defaultSearchEngine);
    connect(cmb_defaultSearchEngine, SIGNAL(activated(const QString &)), this,
            SLOT(moduleChanged()));

    QString wtstr = i18n("Select a search engine to be used for looking up "
                         "normal words or phrases when they are typed into "
                         "applications that have built-in support for this "
                         "feature. To disable it, simply select <em>None</em> "
                         "from the list.");
    QWhatsThis::add(lb_defaultSearchEngine, wtstr);
    QWhatsThis::add(cmb_defaultSearchEngine, wtstr);

    // Deals with the web short cut features...
    gb_autoWebSearch = new QGroupBox (i18n("Web Shortcuts"), this );
    // lay->setStretchFactor( gb_autoWebSearch, 10 );
    gb_autoWebSearch->setColumnLayout( 0, Qt::Horizontal );
    QGridLayout *w_grid = new QGridLayout( gb_autoWebSearch->layout(), 3, 2,
                                           KDialog::spacingHint() );
    w_grid->setColStretch(0, 2);
    w_grid->setRowStretch(2, 2);

    cb_enableWebShortcuts = new QCheckBox(i18n("&Enable"), gb_autoWebSearch);
    connect(cb_enableWebShortcuts, SIGNAL(clicked()), this, SLOT(setWebShortcutState()));
    QWhatsThis::add(cb_enableWebShortcuts, i18n("Enable shortcuts for quickly searching on "
                                                "the web. For example, entering the shortcut "
                                                "<em>gg:KDE</em> will result in a lookup of "
                                                "the word <em>KDE</em> on the Google(TM) "
                                                "search engine."));
    w_grid->addMultiCellWidget(cb_enableWebShortcuts, 0, 0, 0, 1);
    lv_searchProviders = new QListView( gb_autoWebSearch );
    lv_searchProviders->setMultiSelection(false);
    lv_searchProviders->addColumn(i18n("Name"));
    lv_searchProviders->addColumn(i18n("Shortcuts"));
    lv_searchProviders->setSorting(0);
    wtstr = i18n("List of search providers and their associated "
                 "shortcuts.");
    QWhatsThis::add(lv_searchProviders, wtstr);

    connect(lv_searchProviders, SIGNAL(selectionChanged(QListViewItem *)),
           this, SLOT(updateSearchProvider()));
    connect(lv_searchProviders, SIGNAL(doubleClicked(QListViewItem *)),
           this, SLOT(changeSearchProvider()));
    connect(lv_searchProviders, SIGNAL(returnPressed(QListViewItem *)),
           this, SLOT(changeSearchProvider()));
    w_grid->addMultiCellWidget(lv_searchProviders, 1, 2, 0, 0);

    vbox = new QVBox( gb_autoWebSearch );
    vbox->setSpacing( KDialog::spacingHint() );
    pb_addSearchProvider = new QPushButton( i18n("&New..."), vbox );
    QWhatsThis::add(pb_addSearchProvider, i18n("Add a search provider."));
    connect(pb_addSearchProvider, SIGNAL(clicked()), this, SLOT(addSearchProvider()));

    pb_chgSearchProvider = new QPushButton( i18n("Chan&ge..."), vbox );
    QWhatsThis::add(pb_chgSearchProvider, i18n("Modify a search provider."));
    pb_chgSearchProvider->setEnabled(false);
    connect(pb_chgSearchProvider, SIGNAL(clicked()), this, SLOT(changeSearchProvider()));

    pb_delSearchProvider = new QPushButton( i18n("De&lete"), vbox );
    QWhatsThis::add(pb_delSearchProvider, i18n("Delete the selected search provider."));
    pb_delSearchProvider->setEnabled(false);
    connect(pb_delSearchProvider, SIGNAL(clicked()), this, SLOT(deleteSearchProvider()));

#if 0
    pb_impSearchProvider = new QPushButton( i18n("Import..."), vbox );
    QWhatsThis::add(pb_delSearchProvider, i18n("Click here to import a search provider from a file."));
    connect(pb_impSearchProvider, SIGNAL(clicked()), this, SLOT(importSearchProvider()));

    pb_expSearchProvider = new QPushButton(i18n("Export..."), vbox );
    QWhatsThis::add(pb_expSearchProvider, i18n("Click here to export a search provider to a file."));
    pb_expSearchProvider->setEnabled(false);
    connect(pb_expSearchProvider, SIGNAL(clicked()), this, SLOT(exportSearchProvider()));
#endif

    w_grid->addWidget(vbox, 1, 1);
    // Load the options
    load();
}

QString FilterOptions::quickHelp() const
{
    return i18n("In this module you can configure the web shortcuts feature. "
                "Web shortcuts allow you to quickly search or lookup words on "
                "the Internet. For example, to search for information about the "
                "KDE project using the Google engine, you simply type <b>gg:KDE</b> "
                "or <b>google:KDE</b>."
                "<p>If you select a default search engine, normal words or phrases "
                "will be looked up at the specified search engine by simply typing "
                "them into applications, such as Konqueror, that have built-in support "
                "for such a feature.");
}

void FilterOptions::load()
{
    // Clear state first.
    lv_searchProviders->clear();

    cmb_defaultSearchEngine->clear();

    cmb_defaultSearchEngine->insertItem (i18n("None"), 0);

    KConfig config( KURISearchFilterEngine::self()->name() + "rc", false, false );
    config.setGroup("General");

    QString defaultSearchEngine = config.readEntry("DefaultSearchEngine");

    const KTrader::OfferList services = KTrader::self()->query("SearchProvider");

    for (KTrader::OfferList::ConstIterator it = services.begin(); it != services.end(); ++it)
    {
      displaySearchProvider(new SearchProvider(*it),
                            (*it)->desktopEntryName() == defaultSearchEngine);
    }

    // Enable/Disable widgets accordingly.
    bool webShortcutsEnabled = config.readBoolEntry("EnableWebShortcuts", true);
    cb_enableWebShortcuts->setChecked( webShortcutsEnabled );
    setWebShortcutState();
    if (lv_searchProviders->childCount())
      lv_searchProviders->setSelected(lv_searchProviders->firstChild(), true);
}

void FilterOptions::save()
{
  KConfig config( KURISearchFilterEngine::self()->name() + "rc", false, false );

  config.setGroup("General");
  config.writeEntry("EnableWebShortcuts", cb_enableWebShortcuts->isChecked());

  QString engine;

  if (cmb_defaultSearchEngine->currentItem() == 0)
    engine = QString::null;
  else
    engine = cmb_defaultSearchEngine->currentText();

  config.writeEntry("DefaultSearchEngine", m_defaultEngineMap[engine]);

  kdDebug () << "Engine: " << m_defaultEngineMap[engine] << endl;

  QString path = kapp->dirs()->saveLocation("services", "searchproviders/");

  for (QListViewItemIterator it(lv_searchProviders); it.current(); ++it)
  {
    SearchProviderItem *item = dynamic_cast<SearchProviderItem *>(it.current());

    Q_ASSERT(item);

    SearchProvider *provider = item->provider();

    QString name = provider->desktopEntryName();

    if (provider->isDirty())
    {
      if (name.isEmpty())
      {
        // New provider
        // Take the longest search shortcut as filename,
        // if such a file already exists, append a number and increase it
        // until the name is unique
        for (QStringList::ConstIterator it = provider->keys().begin(); it != provider->keys().end(); ++it)
        {
            if ((*it).length() > name.length())
                name = (*it).lower();
        }
        for (int suffix = 0; ; ++suffix)
        {
            QString located, check = name;
            if (suffix)
                check += QString().setNum(suffix);
            if ((located = locate("services", "searchproviders/" + check + ".desktop")).isEmpty())
            {
                name = check;
                break;
            }
            else if (located.left(path.length()) == path)
            {
                // If it's a deleted (hidden) entry, overwrite it
                if (KService(located).isDeleted())
                    break;
            }
        }
      }

      KSimpleConfig service(path + name + ".desktop");
      service.setGroup("Desktop Entry");
      service.writeEntry("Type", "Service");
      service.writeEntry("ServiceTypes", "SearchProvider");
      service.writeEntry("Name", provider->name());
      service.writeEntry("Query", provider->query());
      service.writeEntry("Keys", provider->keys());
      service.writeEntry("Charset", provider->charset());

      // we might be overwriting a hidden entry
      service.writeEntry("Hidden", false);
    }
  }

  for (QStringList::ConstIterator it = m_deletedProviders.begin();
      it != m_deletedProviders.end(); ++it)
  {
      QStringList matches = kapp->dirs()->findAllResources("services", "searchproviders/" + *it + ".desktop");

      // Shouldn't happen
      if (!matches.count())
          continue;

      if (matches.count() == 1 && matches[0].left(path.length()) == path)
      {
          // If only the local copy existed, unlink it
          // TODO: error handling
          QFile::remove(matches[0]);
          continue;
      }
      KSimpleConfig service(path + *it + ".desktop");
      service.setGroup("Desktop Entry");
      service.writeEntry("Type", "Service");
      service.writeEntry("ServiceTypes", "SearchProvider");
      service.writeEntry("Hidden", true);
  }

  config.sync();

  QByteArray data;
  kapp->dcopClient()->send("*", "KURIIKWSFilterIface", "configure()", data);
  kapp->dcopClient()->send("*", "KURISearchFilterIface", "configure()", data);
  kapp->dcopClient()->send( "kded", "kbuildsycoca", "recreate()", data);
}

void FilterOptions::defaults()
{
  load();
}

void FilterOptions::moduleChanged()
{
  // Removed the bool parameter, this way this can be directly connected
  // as it was alwayw called with true as argument anyway (malte)
  emit changed(true);
}

void FilterOptions::setAutoWebSearchState()
{
  moduleChanged();
}

void FilterOptions::setWebShortcutState()
{
  bool use_keywords = cb_enableWebShortcuts->isChecked();
  lv_searchProviders->setEnabled(use_keywords);
  pb_addSearchProvider->setEnabled(use_keywords);
  pb_chgSearchProvider->setEnabled(use_keywords);
  pb_delSearchProvider->setEnabled(use_keywords);
  //pb_impSearchProvider->setEnabled(use_keywords);
  //pb_expSearchProvider->setEnabled(use_keywords);
  moduleChanged();
}

void FilterOptions::addSearchProvider()
{
  SearchProviderDialog dlg(0, this);
  if (dlg.exec())
  {
      lv_searchProviders->setSelected(displaySearchProvider(dlg.provider()), true);
      moduleChanged();
  }
}

void FilterOptions::changeSearchProvider()
{
  SearchProviderItem *item = dynamic_cast<SearchProviderItem *>(lv_searchProviders->currentItem());
  Q_ASSERT(item);

  SearchProviderDialog dlg(item->provider(), this);

  if (dlg.exec())
  {
    lv_searchProviders->setSelected(displaySearchProvider(dlg.provider()), true);
    moduleChanged();
  }
}

void FilterOptions::deleteSearchProvider()
{
  SearchProviderItem *item = dynamic_cast<SearchProviderItem *>(lv_searchProviders->currentItem());
  Q_ASSERT(item);

  // Update the combo box to go to None if the fallback was deleted.
  int current = cmb_defaultSearchEngine->currentItem();
  for (int i = 1, count = cmb_defaultSearchEngine->count(); i < count; ++i)
  {
    if (cmb_defaultSearchEngine->text(i) == item->provider()->name())
    {
      cmb_defaultSearchEngine->removeItem(i);
      if (i == current)
        cmb_defaultSearchEngine->setCurrentItem(0);
      else if (current > i)
        cmb_defaultSearchEngine->setCurrentItem(current - 1);

      break;
    }
  }

  if (item->nextSibling())
      lv_searchProviders->setSelected(item->nextSibling(), true);
  else if (item->itemAbove())
      lv_searchProviders->setSelected(item->itemAbove(), true);

  if (!item->provider()->desktopEntryName().isEmpty())
      m_deletedProviders.append(item->provider()->desktopEntryName());

  delete item;
  updateSearchProvider();
  moduleChanged();
}

void FilterOptions::updateSearchProvider()
{
  pb_chgSearchProvider->setEnabled(lv_searchProviders->currentItem());
  pb_delSearchProvider->setEnabled(lv_searchProviders->currentItem());
  //pb_expSearchProvider->setEnabled(lv_searchProviders->currentItem());
}

SearchProviderItem *FilterOptions::displaySearchProvider(SearchProvider *p, bool fallback)
{
  // Show the provider in the list.
  SearchProviderItem *item = 0L;

  QListViewItemIterator it(lv_searchProviders);

  for (; it.current(); ++it)
  {
    if (it.current()->text(0) == p->name())
    {
      item = dynamic_cast<SearchProviderItem *>(it.current());
      Q_ASSERT(item);
      break;
    }
  }

  if (item)
    item->update ();
  else
  {
    // Put the name in the default search engine combo box.
    int itemCount;
    int totalCount = cmb_defaultSearchEngine->count();

    item = new SearchProviderItem(lv_searchProviders, p);

    for (itemCount = 1; itemCount < totalCount; itemCount++)
    {
      if (cmb_defaultSearchEngine->text(itemCount) > p->name())
      {
        int currentItem = cmb_defaultSearchEngine->currentItem();
        cmb_defaultSearchEngine->insertItem(p->name(), itemCount);
        m_defaultEngineMap[p->name ()] = p->desktopEntryName ();
        if (currentItem >= itemCount)
          cmb_defaultSearchEngine->setCurrentItem(currentItem+1);
        break;
      }
    }

    // Append it to the end of the list...
    if (itemCount == totalCount)
    {
      cmb_defaultSearchEngine->insertItem(p->name(), itemCount);
      m_defaultEngineMap[p->name ()] = p->desktopEntryName ();
    }

    if (fallback)
      cmb_defaultSearchEngine->setCurrentItem(itemCount);
  }

  if (!it.current())
    lv_searchProviders->sort();

  return item;
}

#include "ikwsopts.moc"
