/*
    Copyright 2007 Pino Toscano <pino@kde.org>
    Copyright 2007 Robert Knight <robertknight@gmail.com>

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

// Own
#include "applicationmodel.h"

// Qt
#include <QtCore/QtAlgorithms>
#include <QtCore/QList>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

// KDE
#include <kauthorized.h>
#include <khistorycombobox.h>
#include <kdesktopfile.h>
#include <klineedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kmimetypetrader.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kmimetype.h>
#include <kservicegroup.h>
#include <kdebug.h>

#include <assert.h>
#include <stdlib.h>
#include <kbuildsycocaprogressdialog.h>
#include <kconfiggroup.h>
#include "kickoffadaptor.h"
// Local
#include "core/models.h"

template <> inline
void KConfigGroup::writeEntry( const char *pKey,
                              const KGlobalSettings::Completion& aValue,
                              KConfigBase::WriteConfigFlags flags)
{
  writeEntry(pKey, int(aValue), flags);
}

namespace Kickoff
{

class AppNode
{
public:
    AppNode()
        : isDir(false), parent(0), fetched(false)
    {
    }
    ~AppNode()
    {
        qDeleteAll(children);
    }

    QIcon icon;
    QString genericName;
    QString appName;
    QString relPath;
    QString desktopEntry;
    bool isDir;

    AppNode *parent;
    bool fetched;

    QList<AppNode*> children;
};

class ApplicationModelPrivate
{
public:
    ApplicationModelPrivate(ApplicationModel *qq)
        : q(qq),
          root(new AppNode()),
          duplicatePolicy(ApplicationModel::ShowDuplicatesPolicy),
          sortOrder(Qt::AscendingOrder),
          sortColumn(Qt::DisplayRole)
    {
    }

    ~ApplicationModelPrivate()
    {
        delete root;
    }

    static bool AppNodeLessThan(AppNode *n1, AppNode *n2);
    void fillNode(const QString &relPath, AppNode *node);
    static QHash<QString,QString> iconNameMap();

    ApplicationModel *q;
    AppNode *root;
    ApplicationModel::DuplicatePolicy duplicatePolicy;
    Qt::SortOrder sortOrder;
    int sortColumn;
};

bool ApplicationModelPrivate::AppNodeLessThan(AppNode *n1, AppNode *n2)
{
    if (n1->isDir != n2->isDir) {
        return n1->isDir;
    }

    const QString s1 = n1->genericName.isEmpty() ? n1->appName : n1->genericName;
    const QString s2 = n2->genericName.isEmpty() ? n2->appName : n2->genericName;

    return s1.compare(s2, Qt::CaseInsensitive) < 0;
}

void ApplicationModelPrivate::fillNode(const QString &_relPath, AppNode *node)
{
   KServiceGroup::Ptr root = KServiceGroup::group(_relPath);
   if (!root || !root->isValid()) return;

   KServiceGroup::List list = root->entries();

   // application name <-> service map for detecting duplicate entries
   QHash<QString,KService::Ptr> existingServices;
   for (KServiceGroup::List::ConstIterator it = list.begin();
        it != list.end(); ++it)
   {
      QString icon;
      QString appName;
      QString genericName;
      QString relPath = _relPath;
      QString desktopEntry;
      bool isDir = false;
      const KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
         const KService::Ptr service = KService::Ptr::staticCast(p);

         if (service->noDisplay())
            continue;

         icon = service->icon();
         appName = service->name();
         genericName = service->genericName();
         desktopEntry = service->entryPath();

         // check for duplicates (eg. KDE 3 and KDE 4 versions of application
         // both present)
         if (duplicatePolicy == ApplicationModel::ShowLatestOnlyPolicy &&
             existingServices.contains(appName)
            ) {
                if (Kickoff::isLaterVersion(existingServices[appName],service)) {
                    continue;
                } else {
                    // find and remove the existing entry with the same name
                    for (int i = 0 ; i < node->children.count() ; i++) {
                        if ( node->children[i]->appName == appName ) {
                            delete node->children.takeAt(i);
                        }
                    }
                }
         }
         existingServices[appName] = service;
      }
      else if (p->isType(KST_KServiceGroup))
      {
         const KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);

         if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0)
            continue;

         kDebug(250) << "Service group" << serviceGroup->entryPath() << serviceGroup->icon()
             << serviceGroup->relPath() << serviceGroup->directoryEntryPath();

         icon = serviceGroup->icon();
         if (iconNameMap().contains(icon)) {
            icon = iconNameMap().value(icon);
         }

         genericName = serviceGroup->caption();
         relPath = serviceGroup->relPath();
         appName = serviceGroup->comment();
         isDir = true;
      }
      else
      {
         kWarning(250) << "KServiceGroup: Unexpected object in list!";
         continue;
      }

      AppNode *newnode = new AppNode();
      newnode->icon = KIcon(icon);
      newnode->appName = appName;
      newnode->genericName = genericName;
      newnode->relPath = relPath;
      newnode->desktopEntry = desktopEntry;
      newnode->isDir = isDir;
      newnode->parent = node;
      node->children.append(newnode);
   }

   qStableSort(node->children.begin(), node->children.end(), ApplicationModelPrivate::AppNodeLessThan);
}

ApplicationModel::ApplicationModel(QObject *parent)
    : KickoffAbstractModel(parent), d(new ApplicationModelPrivate(this))
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    (void)new KickoffAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/kickoff", this);
    dbus.connect(QString(), "/kickoff", "org.kde.plasma", "reloadMenu", this, SLOT(slotReloadMenu()));
    d->fillNode(QString(), d->root);
}

ApplicationModel::~ApplicationModel()
{
    delete d;
}

bool ApplicationModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return false;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->isDir && !node->fetched;
}

int ApplicationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        if (!node->genericName.isEmpty()) {
            return node->genericName;
        } else {
            return node->appName;
        }
        break;
    case Kickoff::SubTitleRole:
        if(!node->genericName.isEmpty()) {
            return node->appName;
        }
        break;
    case Kickoff::UrlRole:
        return node->desktopEntry;
    case Qt::DecorationRole:
        return node->icon;
        break;
    default:
        ;
    }
    return QVariant();
}

void ApplicationModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid())
        return;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    if (!node->isDir)
        return;

    emit layoutAboutToBeChanged();
    d->fillNode(node->relPath, node);
    node->fetched = true;
    emit layoutChanged();
}

bool ApplicationModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return true;

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->isDir;
}

QVariant ApplicationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return i18n("Known Applications");
        break;
    default:
        return QVariant();
    }
}

QModelIndex ApplicationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0)
        return QModelIndex();

    AppNode *node = d->root;
    if (parent.isValid())
        node = static_cast<AppNode*>(parent.internalPointer());

    if (row >= node->children.count())
        return QModelIndex();
    else
        return createIndex(row, 0, node->children.at(row));
}

QModelIndex ApplicationModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    if (node->parent->parent) {
        int id = node->parent->parent->children.indexOf(node->parent);

        if (id >= 0 && id < node->parent->parent->children.count())
           return createIndex(id, 0, node->parent);
        else
            return QModelIndex();
    }
    else
        return QModelIndex();
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return d->root->children.count();

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->children.count();
}

void ApplicationModel::setDuplicatePolicy(DuplicatePolicy policy)
{
    delete d->root;
    d->duplicatePolicy = policy;
    d->root = new AppNode();
    d->fillNode(QString(), d->root);
    reset();
}

void ApplicationModel::slotReloadMenu()
{
    delete d->root;
    d->root = new AppNode();
    d->fillNode(QString(), d->root);
    reset();
}

ApplicationModel::DuplicatePolicy ApplicationModel::duplicatePolicy() const
{
    return d->duplicatePolicy;
}

/**
 * FIXME This is a temporary workaround to map the icon names found
 * in the desktop directory files (from /usr/share/desktop-directories)
 * into the Oxygen icon names.  (Only applies if the Gnome menu files
 * are also installed)
 *
 * This list was compiled from Kubuntu 7.04 with the gnome-menus
 * package present.
 *
 * This needs to be discussed on kde-core-devel and fixed
 */
QHash<QString,QString> ApplicationModelPrivate::iconNameMap()
{
    static QHash<QString,QString> map;
    if (map.isEmpty()) {
        map.insert("gnome-util","applications-accessories");
        // accessibility Oxygen icon was missing when this list was compiled
        map.insert("accessibility-directory","applications-other");
        map.insert("gnome-devel","applications-development");
        map.insert("package_edutainment","applications-education");
        map.insert("gnome-joystick","applications-games");
        map.insert("gnome-graphics","applications-graphics");
        map.insert("gnome-globe","applications-internet");
        map.insert("gnome-multimedia","applications-multimedia");
        map.insert("gnome-applications","applications-office");
        map.insert("gnome-system","applications-system");
    }
    return map;
}

} // namespace Kickoff


#include "applicationmodel.moc"
