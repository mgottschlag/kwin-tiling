/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008-2009 Sebastian Sauer <mail@dipe.org>
    Copyright 2009 Christian Loose <christian.loose@kdemail.net>

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
#include "simpleapplet/simpleapplet.h"
#include "simpleapplet/menuview.h"

// Qt
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGraphicsView>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaEnum>
#include <QtCore/QWeakPointer>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>

// KDE Libs
#include <KActionCollection>
#include <KAuthorized>
#include <KBookmarkMenu>
#include <KCModuleInfo>
#include <KComboBox>
#include <KConfigDialog>
#include <KIcon>
#include <KIconButton>
#include <KIconLoader>
#include <KMenu>
#include <KProcess>
#include <KRun>
#include <KServiceTypeTrader>
#include <KToolInvocation>
#include <Solid/PowerManagement>

// KDE Base
#include <kworkspace/kworkspace.h>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/ToolTipManager>

// Local
#include "core/itemhandlers.h"
#include "core/models.h"
#include "core/applicationmodel.h"
#include "core/favoritesmodel.h"
#include "core/systemmodel.h"
#include "core/recentlyusedmodel.h"
#include "core/recentapplications.h"
#include "core/leavemodel.h"
#include "core/urlitemlauncher.h"
#include "ui/contextmenufactory.h"

#ifndef KDE_USE_FINAL
Q_DECLARE_METATYPE(QPersistentModelIndex)
#endif

/// @internal KBookmarkOwner specialization
class BookmarkOwner : public KBookmarkOwner
{
public:
    BookmarkOwner() : KBookmarkOwner() {}
    virtual bool enableOption(BookmarkOption) const {
        return false;
    }
    virtual bool supportsTabs() const {
        return false;
    }
    virtual void openBookmark(const KBookmark& b, Qt::MouseButtons, Qt::KeyboardModifiers) {
        new KRun(b.url(), (QWidget*)0);
    }
};

/// @internal d-pointer class
class MenuLauncherApplet::Private
{
public:
    MenuLauncherApplet *q;

    QWeakPointer<Kickoff::MenuView> menuview;
    Plasma::IconWidget *icon;
    QString iconname;
    QWeakPointer<Kickoff::UrlItemLauncher> launcher;

    KActionCollection* bookmarkcollection;
    BookmarkOwner* bookmarkowner;
    KBookmarkMenu* bookmarkmenu;

    QStringList viewtypes;//QList<MenuLauncherApplet::ViewType>
    QString relativePath;
    MenuLauncherApplet::FormatType formattype;
    int maxRecentApps;
    bool showMenuTitles;

    QListWidget *view;
    KIconButton *iconButton;
    KComboBox *formatComboBox;
    QSpinBox *recentApplicationsSpinBox;
    QCheckBox *showMenuTitlesCheckBox;

    QList<QAction*> actions;
    QAction* switcher;
    Kickoff::ContextMenuFactory *contextMenuFactory;

    bool delayedConfigLoad;

    explicit Private(MenuLauncherApplet *q)
            : q(q),
              icon(0),
              bookmarkcollection(0),
              bookmarkowner(0),
              bookmarkmenu(0),
              view(0),
              iconButton(0),
              formatComboBox(0),
              showMenuTitlesCheckBox(0),
              switcher(0),
              contextMenuFactory(0)
    {}

    ~Private()
    {
        delete bookmarkmenu;
        delete bookmarkowner;
        delete menuview.data();
    }

    void setMaxRecentApps(int num) {
        maxRecentApps = qMax(0, num);
        if (maxRecentApps > Kickoff::RecentApplications::self()->maximum()) {
            Kickoff::RecentApplications::self()->setMaximum(maxRecentApps);
        }
    }

    void addItem(KComboBox* combo, const QString& caption, int index, const QString& icon = QString())
    {
        if (icon.isEmpty()) {
            combo->addItem(caption, index);
        } else {
            combo->addItem(KIcon(icon), caption, index);
        }
    }

    void setCurrentItem(KComboBox* combo, int currentIndex)
    {
        for (int i = combo->count() - 1; i >= 0; --i) {
            if (combo->itemData(i).toInt() == currentIndex) {
                combo->setCurrentIndex(i);
                return;
            }
        }
        if (combo->count() > 0)
            combo->setCurrentIndex(0);
    }

    void addModel(QAbstractItemModel *model, ViewType viewtype, Kickoff::MenuView::ModelOptions options = Kickoff::MenuView::MergeFirstLevel, int formattype = -1)
    {
        Kickoff::MenuView* mainView = menuview.data();
        Kickoff::MenuView* m = mainView;
        if (viewtypes.count() > 1 || !m) {
            m = new Kickoff::MenuView(mainView, viewText(viewtype), KIcon(viewIcon(viewtype)));
            m->setFormatType((formattype >= 0 || !mainView) ? (Kickoff::MenuView::FormatType)formattype : mainView->formatType());
            mainView->addMenu(m);
        }
        m->addModel(model, options);
    }

    QString viewText(MenuLauncherApplet::ViewType vt) const {
        switch (vt) {
            case Favorites:                return i18n("Favorites");
            case Bookmarks:                return i18n("Bookmarks");
            case Applications:             return i18n("Applications");
            case Computer:                 return i18n("Computer");
            case RecentlyUsed:             return i18n("Recently Used");
            case RecentlyUsedApplications: return i18n("Recently Used Applications");
            case RecentlyUsedDocuments:    return i18n("Recently Used Documents");
            case Settings:                 return i18n("System Settings");
            case RunCommand:               return i18n("Run Command...");
            case SwitchUser:               return i18n("Switch User");
            case SaveSession:              return i18n("Save Session");
            case LockScreen:               return i18n("Lock Screen");
            case Standby:                  return i18nc("Puts the system on standby", "Standby");
            case SuspendDisk:              return i18n("Hibernate");
            case SuspendRAM:               return i18n("Sleep");
            case Restart:                  return i18nc("Restart Computer", "Restart");
            case Shutdown:                 return i18n("Shut down");
            case Logout:                   return i18n("Log out");
            case Leave:                    return i18n("Leave");
        }
        return QString();
    }

    QString viewIcon(MenuLauncherApplet::ViewType vt) const {
        switch (vt) {
            case Favorites:                return "bookmarks";
            case Bookmarks:                return "folder-bookmarks";
            case Applications:             return "applications-other";
            case Computer:                 return "computer";
            case RecentlyUsed:             return "document-open-recent";
            case RecentlyUsedApplications: return "document-open-recent";
            case RecentlyUsedDocuments:    return "document-open-recent";
            case Settings:                 return "preferences-system";
            case RunCommand:               return "system-run";
            case SwitchUser:               return "system-switch-user";
            case SaveSession:              return "document-save";
            case LockScreen:               return "system-lock-screen";
            case Standby:                  return "system-suspend";
            case SuspendDisk:              return "system-suspend-hibernate";
            case SuspendRAM:               return "system-suspend-hibernate";
            case Restart:                  return "system-reboot";
            case Shutdown:                 return "system-shutdown";
            case Logout:                   return "system-log-out";
            case Leave:                    return "system-shutdown";
        }
        return QString();
    }

    MenuLauncherApplet::ViewType viewType(const QByteArray& type) const {
        QMetaEnum e = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewType"));
        return (MenuLauncherApplet::ViewType) e.keyToValue(type);
    }

    /*
    QByteArray viewType(MenuLauncherApplet::ViewType type) const {
        QMetaEnum e = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewType"));
        return e.valueToKey(types);
    }
    QList<MenuLauncherApplet::ViewType> viewTypes(const QStringList &types) const {
        QList<MenuLauncherApplet::ViewType> l;
        foreach(QString t, types) l << viewType(t.toUtf());
        return l;
    }
    QStringList viewTypes(const QList<MenuLauncherApplet::ViewType> &types) const {
        QStringList l;
        foreach(MenuLauncherApplet::ViewType t, types) l << viewType(t);
        return l;
    }
    */

    void updateTooltip() {
        QStringList names;
        foreach(const QString &vtname, viewtypes)
            names << viewText(viewType(vtname.toUtf8()));
        Plasma::ToolTipContent data(i18n("Application Launcher Menu"), names.join(", "), icon->icon());
        Plasma::ToolTipManager::self()->setContent(q, data);
    }
};

MenuLauncherApplet::MenuLauncherApplet(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
        d(new Private(this))
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");

    setAspectRatioMode(Plasma::ConstrainedSquare);
    setHasConfigurationInterface(true);
    setBackgroundHints(NoBackground);

    resize(IconSize(KIconLoader::Desktop) * 2, IconSize(KIconLoader::Desktop) * 2);

    d->icon = new Plasma::IconWidget(QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(showMenu(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleMenu()));

    d->delayedConfigLoad = false;
    switch(args.count()) {
        case 2: { //Use submenu paths
            d->viewtypes << "Applications";
            d->relativePath = args.value(0).toString();
            d->iconname = args.value(1).toString();
            break;
        }
        case 1: { //Check for delayed config (switch from Kickoff)
            d->delayedConfigLoad = true;
            //Do not "break;", as we may need the default views configuration
            //(if SimpleLauncher was never used before)
        }
        default: { //Default configuration
            d->viewtypes << "RecentlyUsedApplications" << "Applications" << "Favorites";
            if (KAuthorized::authorize("run_command")) {
                d->viewtypes << "RunCommand";
            }
            d->viewtypes << "Leave";
            d->iconname = "start-here-kde";
        }
    }
    d->formattype = NameDescription;

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(d->icon);

    d->contextMenuFactory = new Kickoff::ContextMenuFactory(this);
    d->contextMenuFactory->setApplet(this);
}

MenuLauncherApplet::~MenuLauncherApplet()
{
    delete d;
}

void MenuLauncherApplet::init()
{
    bool receivedArgs = false;
    if (!d->relativePath.isEmpty()) {
        receivedArgs = true;
    }

    configChanged();

    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ExtensionHandler, "desktop", new Kickoff::ServiceItemHandler);
    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ProtocolHandler, "leave", new Kickoff::LeaveItemHandler);

    if (KService::serviceByStorageId("kde4-kmenuedit.desktop") && KAuthorized::authorize("action/menuedit")) {
        QAction* menueditor = new QAction(i18n("Edit Applications..."), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT(! d->switcher);
    d->switcher = new QAction(i18n("Switch to Application Launcher Style"), this);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    if (receivedArgs) {
        KConfigGroup cg = config();
        cg.writeEntry("relativePath", d->relativePath);
        cg.writeEntry("icon", d->iconname);
        emit configNeedsSaving();
    }

    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
        this, SLOT(iconSizeChanged(int)));
}

void MenuLauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
            //FIXME set correct minimum size
            //setMinimumContentSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            //setMinimumContentSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
        }
    }

    if ((constraints & Plasma::ImmutableConstraint) && d->switcher) {
        d->switcher->setVisible(immutability() == Plasma::Mutable);
    }
}

void MenuLauncherApplet::switchMenuStyle()
{
    if (containment()) {
        Plasma::Applet *launcher = containment()->addApplet("launcher", QVariantList(), geometry());
        //Copy all the config items to the simple launcher
        QMetaObject::invokeMethod(launcher, "saveConfigurationFromSimpleLauncher",
                                  Qt::DirectConnection, Q_ARG(KConfigGroup, config()),
                                  Q_ARG(KConfigGroup, globalConfig()));

        //Switch shortcuts with the new launcher to avoid losing it
        KShortcut currentShortcut = globalShortcut();
        setGlobalShortcut(KShortcut());
        launcher->setGlobalShortcut(currentShortcut);
        destroy();
    }
}

void MenuLauncherApplet::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void MenuLauncherApplet::customContextMenuRequested(QMenu* menu, const QPoint& pos)
{
    if (!menu) {
        return;
    }

    // there are 2 possible cases
    // 1) An an actual action is active - menu is the menu containing the action wanted
    // 2) either a submenu is active, but none of its actions are active. menu is the submenu,
    //    not the menu containing the submenu
    QAction* menuAction = menu->activeAction();
    if (menuAction) { // case 1)
        const QPersistentModelIndex index = menuAction->data().value<QPersistentModelIndex>();
        d->contextMenuFactory->showContextMenu(0, index, pos);
    } else { // case 2)
        menuAction = menu->menuAction();
        if (menuAction) {
            const QPersistentModelIndex index = menuAction->data().value<QPersistentModelIndex>();
            d->contextMenuFactory->showContextMenu(0, index, pos);
        }
    }
}

void MenuLauncherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    d->view = 0;
    d->recentApplicationsSpinBox = 0;
    if (d->relativePath.isEmpty()) {
        QWidget *viewpage = new QWidget(parent);
        QVBoxLayout *l = new QVBoxLayout(viewpage);
        l->setMargin(0);
        viewpage->setLayout(l);
        d->view = new QListWidget(viewpage);
        d->view->resize(300,500);
        l->addWidget(d->view);

        QMetaEnum vte = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
        for(int i = 0; i < vte.keyCount(); ++i) {
            ViewType vt = (ViewType) vte.value(i);
            const QByteArray vtname = vte.key(i);
            QListWidgetItem *item = new QListWidgetItem(KIcon(d->viewIcon(vt)), d->viewText(vt), d->view);
            item->setCheckState(d->viewtypes.contains(vtname) ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole, vtname);
            d->view->addItem(item);
        }
        parent->addPage(viewpage, i18n("View"), "view-list-details");
    }

    QWidget *p = new QWidget(parent);
    QGridLayout *grid = new QGridLayout(p);
    grid->setMargin(0);

    QLabel *iconLabel = new QLabel(i18n("Icon:"), p);
    grid->addWidget(iconLabel, 0, 0, Qt::AlignRight);
    d->iconButton = new KIconButton(p);
    d->iconButton->setIcon(d->icon->icon());
    iconLabel->setBuddy(d->iconButton);
    grid->addWidget(d->iconButton, 0, 1);

    QLabel *formatLabel = new QLabel(i18nc("@label:listbox How to present applications in a KMenu-like menu", "Format:"), p);
    grid->addWidget(formatLabel, 1, 0, Qt::AlignRight);
    d->formatComboBox = new KComboBox(p);
    formatLabel->setBuddy(d->formatComboBox);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Only"), MenuLauncherApplet::Name);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description Only"), MenuLauncherApplet::Description);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name (Description)"), MenuLauncherApplet::NameDescription);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description (Name)"), MenuLauncherApplet::DescriptionName);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name - Description"), MenuLauncherApplet::NameDashDescription);
    d->setCurrentItem(d->formatComboBox, d->formattype);
    grid->addWidget(d->formatComboBox, 1, 1);

    if (d->relativePath.isEmpty()) {
        QLabel *recentLabel = new QLabel(i18n("Recently used applications:"), p);
        grid->addWidget(recentLabel, 2, 0, Qt::AlignRight);
        d->recentApplicationsSpinBox = new QSpinBox(p);
        d->recentApplicationsSpinBox->setMaximum(10);
        d->recentApplicationsSpinBox->setMinimum(0);
        d->recentApplicationsSpinBox->setValue(d->maxRecentApps);
        recentLabel->setBuddy(d->recentApplicationsSpinBox);
        grid->addWidget(d->recentApplicationsSpinBox, 2, 1);
    }

    QLabel *showMenuTitlesLabel = new QLabel(i18n("Show menu titles:"), p);
    grid->addWidget(showMenuTitlesLabel, 3, 0, Qt::AlignRight);
    d->showMenuTitlesCheckBox = new QCheckBox(p);
    d->showMenuTitlesCheckBox->setChecked(d->showMenuTitles);
    grid->addWidget(d->showMenuTitlesCheckBox, 3, 1);

    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0, 1, 3);
    parent->addPage(p, i18n("Options"), "configure");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    connect(d->iconButton, SIGNAL(iconChanged(QString)), parent, SLOT(settingsModified()));
    connect(d->formatComboBox, SIGNAL(currentIndexChanged(QString)), parent, SLOT(settingsModified()));
    connect(d->recentApplicationsSpinBox, SIGNAL(valueChanged(int)), parent, SLOT(settingsModified()));
    connect(d->showMenuTitlesCheckBox, SIGNAL(toggled(bool)), parent, SLOT(settingsModified()));
    connect(d->view, SIGNAL(currentTextChanged(QString)), parent, SLOT(settingsModified()));    
}

void MenuLauncherApplet::configAccepted()
{
    bool needssaving = false;
    KConfigGroup cg = config();

    if (d->view) {
        QStringList viewtypes;
        for(int i = 0; i < d->view->count(); ++i) {
            QListWidgetItem *item = d->view->item(i);
            QByteArray vtname = item->data(Qt::UserRole).toByteArray();
            if(item->checkState() == Qt::Checked)
                viewtypes << vtname;
            if( !needssaving && ((item->checkState() == Qt::Checked && ! d->viewtypes.contains(vtname)) || (item->checkState() == Qt::Unchecked && d->viewtypes.contains(vtname))) )
                needssaving = true;
        }
        if(needssaving) {
            d->viewtypes = viewtypes;
            d->updateTooltip();
            cg.writeEntry("views", d->viewtypes);
            cg.deleteEntry("view"); // "view" was from <KDE4.3, we are using "views" now
        }
    }

    const QString iconname = d->iconButton->icon();
    if (! iconname.isEmpty()) {
        needssaving = true;
        d->icon->setIcon(KIcon(iconname));
        d->updateTooltip();
        cg.writeEntry("icon", iconname);
    }

    const int ft = d->formatComboBox->itemData(d->formatComboBox->currentIndex()).toInt();
    if (ft != d->formattype) {
        needssaving = true;
        d->formattype = (MenuLauncherApplet::FormatType) ft;
        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
        cg.writeEntry("format", QByteArray(e.valueToKey(d->formattype)));
    }

    if (d->recentApplicationsSpinBox) {
        const int maxRecentApps = d->recentApplicationsSpinBox->value();
        if (maxRecentApps != d->maxRecentApps) {
            needssaving = true;
            d->setMaxRecentApps(maxRecentApps);
            cg.writeEntry("maxRecentApps", maxRecentApps);
        }
    }

    const bool showMenuTitles = d->showMenuTitlesCheckBox->isChecked();
    if (showMenuTitles != d->showMenuTitles) {
        needssaving = true;
        d->showMenuTitles = showMenuTitles;
        cg.writeEntry("showMenuTitles", showMenuTitles);
    }

    if (needssaving) {
        d->updateTooltip();
        emit configNeedsSaving();
        if (d->menuview) {
            d->menuview.data()->deleteLater();
        }
    }
}

inline int weightOfService( const KService::Ptr service )
{
    QVariant tmp = service->property("X-KDE-Weight", QVariant::Int);
    return (tmp.isValid() ? tmp.toInt() : 100);
}
inline bool sortServiceItemsByWeight(const KService::Ptr left, const KService::Ptr right)
{
    return weightOfService(left) < weightOfService(right);
}
KService::List sortServices(KService::List list)
{
    qSort(list.begin(), list.end(), sortServiceItemsByWeight);
    return list;
}

void MenuLauncherApplet::toggleMenu()
{
    showMenu(!d->menuview || !d->menuview.data()->isVisible());
}

void MenuLauncherApplet::showMenu(bool pressed)
{
    if (!pressed || d->viewtypes.count()<=0) {
        if (d->menuview) {
            d->menuview.data()->deleteLater();
            d->menuview.clear();
        }

        return;
    }

    d->icon->setPressed();

    if (!d->menuview) {
        Kickoff::MenuView *menuview = new Kickoff::MenuView();
        d->menuview = menuview;
        menuview->setAttribute(Qt::WA_DeleteOnClose);
        menuview->setFormatType( (Kickoff::MenuView::FormatType) d->formattype );
        menuview->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(menuview, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
        connect(menuview, SIGNAL(aboutToHide()), this, SLOT(menuHiding()));
        connect(menuview, SIGNAL(customContextMenuRequested(QMenu*, const QPoint&)),
                this, SLOT(customContextMenuRequested(QMenu*, const QPoint&)));
        //connect(menuview, SIGNAL(afterBeingHidden()), menuview, SLOT(deleteLater()));

        //Kickoff::MenuView::ModelOptions options = d->viewtypes.count() < 2 ? Kickoff::MenuView::MergeFirstLevel : Kickoff::MenuView::None;
        foreach(const QString &vtname, d->viewtypes) {
            if(vtname == "Applications") {
                Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(menuview, true /*allow separators*/);

                appModel->setNameDisplayOrder(Kickoff::NameBeforeDescription);

                appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
                if (d->formattype == Name || d->formattype == NameDescription || d->formattype == NameDashDescription)
                    appModel->setPrimaryNamePolicy(Kickoff::ApplicationModel::AppNamePrimary);
                appModel->setSystemApplicationPolicy(Kickoff::ApplicationModel::ShowApplicationAndSystemPolicy);

                menuview->addModel(appModel, Kickoff::MenuView::None, d->relativePath);

                if (d->relativePath.isEmpty()) {
                    if (d->showMenuTitles) {
                        menuview->setModelTitleVisible(appModel, true);
                        menuview->addTitle(i18n("Actions"));
                    } else {
                        menuview->addSeparator();
                    }
                }
            } else if(vtname == "Favorites") {
                d->addModel(new Kickoff::FavoritesModel(menuview), Favorites);
            } else if(vtname == "Computer") {
                d->addModel(new Kickoff::SystemModel(menuview), Computer);
            } else if(vtname == "RecentlyUsed") {
                d->addModel(new Kickoff::RecentlyUsedModel(menuview), RecentlyUsed);
            } else if(vtname == "RecentlyUsedApplications") {
                if (d->maxRecentApps > 0) {
                    Kickoff::RecentlyUsedModel *recentModel = new Kickoff::RecentlyUsedModel(menuview, Kickoff::RecentlyUsedModel::ApplicationsOnly, d->maxRecentApps);
                    menuview->addModel(recentModel, Kickoff::MenuView::MergeFirstLevel);

                    if (d->showMenuTitles) {
                        menuview->setModelTitleVisible(recentModel, true);
                    } else {
                        menuview->addSeparator();
                    }
                }
            } else if(vtname == "RecentlyUsedDocuments") {
                Kickoff::RecentlyUsedModel *recentModel = new Kickoff::RecentlyUsedModel(menuview, Kickoff::RecentlyUsedModel::DocumentsOnly);
                menuview->addModel(recentModel, Kickoff::MenuView::MergeFirstLevel);

                if (d->showMenuTitles) {
                    menuview->setModelTitleVisible(recentModel, true);
                } else {
                    menuview->addSeparator();
                }
            } else if(vtname == "Bookmarks") {
                KMenu* menu = menuview;
                if(d->viewtypes.count() > 1) {
                    menu = new KMenu(d->viewText(Bookmarks), menuview);
                    menu->setIcon(KIcon(d->viewIcon(Bookmarks)));
                    menuview->addMenu(menu);
                }
                KBookmarkManager* mgr = KBookmarkManager::userBookmarksManager();
                if (! d->bookmarkcollection) {
                    d->bookmarkcollection = new KActionCollection(this);
                    d->bookmarkowner = new BookmarkOwner();
                }
                delete d->bookmarkmenu;
                d->bookmarkmenu = new KBookmarkMenu(mgr, d->bookmarkowner, menu, d->bookmarkcollection);
            } else if(vtname == "Settings") {
                KMenu* parentmenu = menuview;
                if(d->viewtypes.count() > 1) {
                    parentmenu = new KMenu(d->viewText(Settings), menuview);
                    parentmenu->setIcon(KIcon(d->viewIcon(Settings)));
                    menuview->addMenu(parentmenu);
                }
                QMap<QString, KMenu*> menus;
                foreach(const KService::Ptr &rootentry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", "(not exist [X-KDE-System-Settings-Parent-Category]) or [X-KDE-System-Settings-Parent-Category]==''"))) {
                    parentmenu->addTitle(rootentry->name().replace('&',"&&"));
                    const QString rootcategory = rootentry->property("X-KDE-System-Settings-Category").toString();
                    foreach(const KService::Ptr &entry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", QString("[X-KDE-System-Settings-Parent-Category]=='%1'").arg(rootcategory)))) {
                        KMenu* menu = new KMenu(entry->name().replace('&',"&&"), parentmenu);
                        menu->setIcon(KIcon(entry->icon()));
                        parentmenu->addMenu(menu);
                        const QString category = entry->property("X-KDE-System-Settings-Category").toString();
                        menus[category] = menu;
                    }
                }
                QMap<QString, QList<KService::Ptr> > modules;
                foreach(const KService::Ptr &entry, sortServices(KServiceTypeTrader::self()->query("KCModule"))) {
                    const QString category = entry->property("X-KDE-System-Settings-Parent-Category").toString();
                    if(! category.isEmpty() && ! entry->noDisplay())
                        modules[category] << entry;
                }
                foreach(const QString& category, modules.keys()) {
                    QString menucategory = category;
                    KService::Ptr subcategory = KService::Ptr();
                    while(! menucategory.isEmpty() && ! menus.contains(menucategory)) {
                        KService::List services = KServiceTypeTrader::self()->query("SystemSettingsCategory", QString("[X-KDE-System-Settings-Category]=='%1'").arg(menucategory));
                        //Q_ASSERT(services.count() > 0); //if that happens then we miss the desktop-file defining the category
                        //Q_ASSERT(services.count() < 2); //if that happens then we have more then one desktop-file defining the same category
                        if(services.count() < 1) {
                            menucategory.clear();
                        } else {
                            subcategory = services[0];
                            menucategory = subcategory->property("X-KDE-System-Settings-Parent-Category").toString();
                        }
                    }
                    if(menucategory.isEmpty()) continue; //skip the category
                    KMenu* m = menus[menucategory];
                    if(! subcategory.isNull())
                        m->addTitle(subcategory->name().replace('&',"&&"));
                    foreach(const KService::Ptr &entry, modules[category]) {
                        KCModuleInfo module(entry->entryPath());
                        m->addAction(KIcon(module.icon()), module.moduleName().replace('&',"&&"))->setData(KUrl("kcm:/" + entry->entryPath()));
                    }
                }
            } else if(vtname == "RunCommand") {
                if (KAuthorized::authorize("run_command")) {
                    menuview->addAction(KIcon(d->viewIcon(RunCommand)), d->viewText(RunCommand))->setData(KUrl("leave:/run"));
                }
            } else if(vtname == "SwitchUser") {
                menuview->addAction(KIcon(d->viewIcon(SwitchUser)), d->viewText(SwitchUser))->setData(KUrl("leave:/switch"));
            } else if(vtname == "SaveSession") {
                KConfigGroup c(KSharedConfig::openConfig("ksmserverrc", KConfig::NoGlobals), "General");
                if (c.readEntry("loginMode") == "restoreSavedSession")
                    menuview->addAction(KIcon(d->viewIcon(SaveSession)), d->viewText(SaveSession))->setData(KUrl("leave:/savesession"));
            } else if(vtname == "LockScreen") {
                menuview->addAction(KIcon(d->viewIcon(LockScreen)), d->viewText(LockScreen))->setData(KUrl("leave:/lock"));
            } else if(vtname == "Logout") {
                menuview->addAction(KIcon(d->viewIcon(Logout)), d->viewText(Logout))->setData(KUrl("leave:/logout"));
            } else if(vtname == "Leave") {
                Kickoff::LeaveModel *leavemodel = new Kickoff::LeaveModel(menuview);
                leavemodel->updateModel();
                d->addModel(leavemodel, Leave, Kickoff::MenuView::MergeFirstLevel, Kickoff::MenuView::Name);
            } else {
#ifndef Q_WS_WIN
                QSet< Solid::PowerManagement::SleepState > spdMethods = Solid::PowerManagement::supportedSleepStates();
                if (vtname == "Standby") {
                    if (spdMethods.contains(Solid::PowerManagement::StandbyState))
                        menuview->addAction(KIcon(d->viewIcon(Standby)), d->viewText(Standby))->setData(KUrl("leave:/standby"));
                } else if(vtname == "SuspendDisk") {
                    if (spdMethods.contains(Solid::PowerManagement::HibernateState))
                        menuview->addAction(KIcon(d->viewIcon(SuspendDisk)), d->viewText(SuspendDisk))->setData(KUrl("leave:/suspenddisk"));
                } else if(vtname == "SuspendRAM") {
                    if (spdMethods.contains(Solid::PowerManagement::SuspendState))
                        menuview->addAction(KIcon(d->viewIcon(SuspendRAM)), d->viewText(SuspendRAM))->setData(KUrl("leave:/suspendram"));
                } else if(vtname == "Restart") {
                    if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeReboot))
                        menuview->addAction(KIcon(d->viewIcon(Restart)), d->viewText(Restart))->setData(KUrl("leave:/restart"));
                } else if(vtname == "Shutdown") {
                    if (KWorkSpace::canShutDown(KWorkSpace::ShutdownConfirmDefault, KWorkSpace::ShutdownTypeHalt))
                        menuview->addAction(KIcon(d->viewIcon(Shutdown)), d->viewText(Shutdown))->setData(KUrl("leave:/shutdown"));
                }
#endif
            }
        }
    }

    Plasma::ToolTipManager::self()->hide(this);
    setStatus(Plasma::NeedsAttentionStatus);
    d->menuview.data()->popup(popupPosition(d->menuview.data()->sizeHint()));
}

void MenuLauncherApplet::menuHiding()
{
    d->icon->setUnpressed();
    setStatus(Plasma::PassiveStatus);
}

void MenuLauncherApplet::actionTriggered(QAction *action)
{
    const KUrl url = action->data().value<KUrl>();
    if (url.scheme() == "leave") {
        if (!d->launcher) {
            d->launcher = new Kickoff::UrlItemLauncher(d->menuview.data());
        }

        d->launcher.data()->openUrl(url.url());
        return;
    }
    if (url.scheme() == "kcm") {
        KToolInvocation::kdeinitExec("kcmshell4",  QStringList() << url.fileName());
        return;
    }
    for (QWidget* w = action->parentWidget(); w; w = w->parentWidget()) {
        if (Kickoff::MenuView *view = dynamic_cast<Kickoff::MenuView*>(w)) {
            view->actionTriggered(action);
            break;
        }
    }
}

QList<QAction*> MenuLauncherApplet::contextualActions()
{
    return d->actions;
}

void MenuLauncherApplet::iconSizeChanged(int group)
{
    if (group == KIconLoader::Desktop || group == KIconLoader::Panel) {
        updateGeometry();
    }
}

QSizeF MenuLauncherApplet::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        int iconSize;

        switch (formFactor()) {
            case Plasma::Planar:
            case Plasma::MediaCenter:
                iconSize = IconSize(KIconLoader::Desktop);
                break;

            case Plasma::Horizontal:
            case Plasma::Vertical:
                iconSize = IconSize(KIconLoader::Panel);
                break;
        }

        return QSizeF(iconSize, iconSize);
    }

    return Plasma::Applet::sizeHint(which, constraint);
}


void MenuLauncherApplet::saveConfigurationFromKickoff(const KConfigGroup & configGroup, const KConfigGroup & globalConfigGroup)
{
    //Copy configuration values
    KConfigGroup cg = config();
    configGroup.copyTo(&cg);

    KConfigGroup gcg = globalConfig();
    globalConfigGroup.copyTo(&gcg);

    configChanged();
    emit configNeedsSaving();
}

void MenuLauncherApplet::configChanged()
{
    KConfigGroup cg = config();

    const QStringList viewtypes = cg.readEntry("views", QStringList());
    if(viewtypes.isEmpty()) { // backward-compatibility to <KDE4.3
        QByteArray oldview = cg.readEntry("view", QByteArray());
        if (!oldview.isEmpty() && oldview != "Combined") {
            d->viewtypes = QStringList() << oldview;
            d->iconname = d->viewIcon(d->viewType(oldview));
        } // else we use the default d->viewtypes
    } else {
        d->viewtypes = viewtypes;
    }

    QMetaEnum fte = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
    QByteArray ftb = cg.readEntry("format", QByteArray(fte.valueToKey(d->formattype)));
    d->formattype = (MenuLauncherApplet::FormatType) fte.keyToValue(ftb);

    d->setMaxRecentApps(cg.readEntry("maxRecentApps", qMin(5, Kickoff::RecentApplications::self()->maximum())));
    d->showMenuTitles = cg.readEntry("showMenuTitles", false);

    d->icon->setIcon(KIcon(cg.readEntry("icon", d->iconname)));

    d->relativePath = cg.readEntry("relativePath", d->relativePath);
    if (!d->relativePath.isEmpty()) {
        d->viewtypes.clear();
        d->viewtypes << "Applications";
    }

    d->updateTooltip();

    constraintsEvent(Plasma::ImmutableConstraint);
}

#include "simpleapplet.moc"
