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
#include <ksycoca.h>
#include <kdebug.h>

#include <assert.h>
#include <stdlib.h>
#include <kbuildsycocaprogressdialog.h>
#include <kconfiggroup.h>
#include "kickoffadaptor.h"
// Local
#include "core/models.h"

template <> inline
void KConfigGroup::writeEntry(const char *pKey,
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
        : parent(0),
          fetched(false),
          isDir(false),
          isSeparator(false),
          subTitleMandatory(false)
    {
    }

    ~AppNode()
    {
        qDeleteAll(children);
    }

    QList<AppNode*> children;

    QIcon icon;
    QString iconName;
    QString genericName;
    QString appName;
    QString relPath;
    QString desktopEntry;

    AppNode *parent;
    DisplayOrder displayOrder;
    bool fetched : 1;
    bool isDir : 1;
    bool isSeparator : 1;
    bool subTitleMandatory : 1;
};

class ApplicationModelPrivate
{
public:
    ApplicationModelPrivate(ApplicationModel *qq, bool _allowSeparators)
            : q(qq),
              root(new AppNode()),
              duplicatePolicy(ApplicationModel::ShowDuplicatesPolicy),
              systemApplicationPolicy(ApplicationModel::ShowApplicationAndSystemPolicy),
              primaryNamePolicy(ApplicationModel::GenericNamePrimary),
              displayOrder(NameAfterDescription),
              allowSeparators(_allowSeparators)
    {
        systemApplications = Kickoff::systemApplicationList();
        reloadTimer = new QTimer(qq);
        reloadTimer->setSingleShot(true);
        QObject::connect(reloadTimer, SIGNAL(timeout()), qq, SLOT(delayedReloadMenu()));
    }

    ~ApplicationModelPrivate()
    {
        delete root;
    }

    void fillNode(const QString &relPath, AppNode *node);
    static QHash<QString, QString> iconNameMap();

    ApplicationModel *q;
    AppNode *root;
    ApplicationModel::DuplicatePolicy duplicatePolicy;
    ApplicationModel::SystemApplicationPolicy systemApplicationPolicy;
    ApplicationModel::PrimaryNamePolicy primaryNamePolicy;
    QStringList systemApplications;
    DisplayOrder displayOrder;
    bool allowSeparators;
    QTimer *reloadTimer;
};

void ApplicationModelPrivate::fillNode(const QString &_relPath, AppNode *node)
{
    KServiceGroup::Ptr root = KServiceGroup::group(_relPath);

    if (!root || !root->isValid()) {
        return;
    }

    const KServiceGroup::List list = root->entries(true /* sorted */,
                                                   true /* exclude no display entries */,
                                                   allowSeparators /* allow separators */,
                                                   primaryNamePolicy == ApplicationModel::GenericNamePrimary /* sort by generic name */);

    // application name <-> service map for detecting duplicate entries
    QHash<QString, KService::Ptr> existingServices;

    // generic name <-> node mapping to determinate duplicate generic names
    QHash<QString,QList<AppNode*> > genericNames;

    for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        QString icon;
        QString appName;
        QString genericName;
        QString relPath = _relPath;
        QString desktopEntry;
        bool isDir = false;
        bool isSeparator = false;
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = KService::Ptr::staticCast(p);

            if (service->noDisplay()) {
                continue;
            }

            icon = service->icon();
            appName = service->name();
            genericName = service->genericName();
            desktopEntry = service->entryPath();

            // check for duplicates (eg. KDE 3 and KDE 4 versions of application
            // both present)
            if (duplicatePolicy == ApplicationModel::ShowLatestOnlyPolicy &&
                existingServices.contains(appName)) {
                if (Kickoff::isLaterVersion(existingServices[appName], service)) {
                    continue;
                } else {
                    // find and remove the existing entry with the same name
                    for (int i = node->children.count() - 1; i >= 0; --i) {
                        AppNode *app = node->children.at(i);
                        if (app->appName == appName &&
                            app->genericName == genericName &&
                            app->iconName == icon) {
                            app = node->children.takeAt(i);
                            const QString s = app->genericName.toLower();
                            if (genericNames.contains(s)) {
                                QList<AppNode*> list = genericNames[s];
                                for (int j = list.count() - 1; j >= 0; --j) {
                                    if(list.at(j) == app) {
                                        list.takeAt(j);
                                    }
                                }
                                genericNames[s] = list;
                            }
                            delete app;
                        }
                    }
                }
            }

            if (systemApplicationPolicy == ApplicationModel::ShowSystemOnlyPolicy &&
                systemApplications.contains(service->desktopEntryName())) {
                // don't duplicate applications that are configured to appear in the System tab
                // in the Applications tab
                continue;
            }

            existingServices[appName] = service;
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);

            if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0) {
                continue;
            }

            kDebug(250) << "Service group" << serviceGroup->entryPath() << serviceGroup->icon()
            << serviceGroup->relPath() << serviceGroup->directoryEntryPath();

            icon = serviceGroup->icon();
            if (iconNameMap().contains(icon)) {
                icon = iconNameMap().value(icon);
            }

            desktopEntry = serviceGroup->entryPath();
            genericName = serviceGroup->caption();
            relPath = serviceGroup->relPath();
            appName = serviceGroup->comment();
            isDir = true;
        } else if (p->isType(KST_KServiceSeparator)) {
            isSeparator = true;
        } else {
            kWarning(250) << "KServiceGroup: Unexpected object in list!";
            continue;
        }

        AppNode *newnode = new AppNode();
        newnode->iconName = icon;
        newnode->icon = KIcon(icon);
        newnode->appName = appName;
        newnode->genericName = genericName;
        newnode->relPath = relPath;
        newnode->desktopEntry = desktopEntry;
        newnode->isDir = isDir;
        newnode->isSeparator = isSeparator;
        newnode->parent = node;
        node->children.append(newnode);

        if (p->isType(KST_KService)) {
            const QString s = genericName.toLower();
            QList<AppNode*> list = genericNames.value(s);
            list.append(newnode);
            genericNames[s] = list;
        }
    }

    // set the subTitleMandatory field for nodes that do not provide a unique generic
    // name what may help us on display to show in such cases also the subtitle to
    // provide a hint to the user what the duplicate entries are about.
    foreach (const QList<AppNode*> &list, genericNames) {
        if (list.count() > 1) {
            foreach (AppNode* n, list) {
                n->subTitleMandatory = true;
            }
        }
    }
}

ApplicationModel::ApplicationModel(QObject *parent, bool allowSeparators)
  : KickoffAbstractModel(parent),
    d(new ApplicationModelPrivate(this, allowSeparators))
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    (void)new KickoffAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/kickoff", this);
    dbus.connect(QString(), "/kickoff", "org.kde.plasma", "reloadMenu", this, SLOT(reloadMenu()));
    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(checkSycocaChange(QStringList)));
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

void ApplicationModel::setNameDisplayOrder(DisplayOrder displayOrder) 
{
    d->displayOrder = displayOrder;
}

DisplayOrder ApplicationModel::nameDisplayOrder() const
{
   return d->displayOrder;
}

int ApplicationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

bool ApplicationModel::nameAfterDescription(const QModelIndex &index) const
{
    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    if (node->isDir) {
        return true;
    }

    QModelIndex parent = index.parent();
    while (parent.parent().isValid()) {
        parent = parent.parent();
    }

    if (parent.isValid()) {
        // nasty little hack to always makes games show their unique name
        // there is no such thing as a "generic name" for a game in practice
        // though this is apparently quite true for all other kinds of apps
        AppNode *node = static_cast<AppNode*>(parent.internalPointer());
        if (node->isDir && node->genericName == i18n("Games")) {
            return false;
        }
    }

    return d->displayOrder == NameAfterDescription;
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    AppNode *node = static_cast<AppNode*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
      if (nameAfterDescription(index) && !node->genericName.isEmpty()) {
            return node->genericName;
        } else {
            return node->appName;
        }
        break;
    case Kickoff::SubTitleRole:
      if (!nameAfterDescription(index) && !node->genericName.isEmpty()) {
            return node->genericName;
        } else {
            return node->appName;
        }
        break;
    case Kickoff::UrlRole:
        if (node->isDir) {
            return QString::fromLatin1("applications://%1").arg(node->desktopEntry);
        } else {
            return node->desktopEntry;
        }
        break;
    case Kickoff::SubTitleMandatoryRole:
        return nameAfterDescription(index) && node->subTitleMandatory;
        break;
    case Kickoff::SeparatorRole:
        return node->isSeparator;
        break;
    case Qt::DecorationRole:
        return node->icon;
        break;
    case Kickoff::RelPathRole:
        return node->relPath;
        break;
    case Kickoff::IconNameRole:
        return node->iconName;
        break;
    default:
        ;
    }
    return QVariant();
}

void ApplicationModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid()) {
        return;
    }

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    if (!node->isDir) {
        return;
    }

    emit layoutAboutToBeChanged();
    d->fillNode(node->relPath, node);
    node->fetched = true;
    emit layoutChanged();
}

bool ApplicationModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return true;
    }

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->isDir;
}

QVariant ApplicationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return i18n("All Applications");
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
    if (!index.isValid()) {
        return QModelIndex();
    }

    AppNode *node = static_cast<AppNode*>(index.internalPointer());
    if (node->parent->parent) {
        int id = node->parent->parent->children.indexOf(node->parent);

        if (id >= 0 && id < node->parent->parent->children.count()) {
            return createIndex(id, 0, node->parent);
        }
    }

    return QModelIndex();
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->root->children.count();
    }

    AppNode *node = static_cast<AppNode*>(parent.internalPointer());
    return node->children.count();
}

void ApplicationModel::setDuplicatePolicy(DuplicatePolicy policy)
{
    if (d->duplicatePolicy != policy) {
        d->duplicatePolicy = policy;
        reloadMenu();
    }
}

void ApplicationModel::setSystemApplicationPolicy(SystemApplicationPolicy policy)
{
    if (d->systemApplicationPolicy != policy) {
        d->systemApplicationPolicy = policy;
        reloadMenu();
    }
}

void ApplicationModel::setPrimaryNamePolicy(PrimaryNamePolicy policy)
{
    if (policy != d->primaryNamePolicy) {
        d->primaryNamePolicy = policy;
        reloadMenu();
    }
}

ApplicationModel::PrimaryNamePolicy ApplicationModel::primaryNamePolicy() const
{
    return d->primaryNamePolicy;
}

void ApplicationModel::delayedReloadMenu()
{
    if (!d->reloadTimer->isActive()) {
        d->reloadTimer->start(200);
    }
}

void ApplicationModel::reloadMenu()
{
    delete d->root;
    d->root = new AppNode();
    d->fillNode(QString(), d->root);
    reset();
}

void ApplicationModel::checkSycocaChange(const QStringList &changes)
{
    if (changes.contains("services") || changes.contains("apps")) {
        reloadMenu();
    }
}

ApplicationModel::DuplicatePolicy ApplicationModel::duplicatePolicy() const
{
    return d->duplicatePolicy;
}

ApplicationModel::SystemApplicationPolicy ApplicationModel::systemApplicationPolicy() const
{
    return d->systemApplicationPolicy;
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
QHash<QString, QString> ApplicationModelPrivate::iconNameMap()
{
    static QHash<QString, QString> map;
    if (map.isEmpty()) {
        map.insert("gnome-util", "applications-accessories");
        // accessibility Oxygen icon was missing when this list was compiled
        map.insert("accessibility-directory", "applications-other");
        map.insert("gnome-devel", "applications-development");
        map.insert("package_edutainment", "applications-education");
        map.insert("gnome-joystick", "applications-games");
        map.insert("gnome-graphics", "applications-graphics");
        map.insert("gnome-globe", "applications-internet");
        map.insert("gnome-multimedia", "applications-multimedia");
        map.insert("gnome-applications", "applications-office");
        map.insert("gnome-system", "applications-system");
    }
    return map;
}

} // namespace Kickoff


#include "applicationmodel.moc"
