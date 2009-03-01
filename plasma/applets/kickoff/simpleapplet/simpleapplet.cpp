/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008-2009 Sebastian Sauer <mail@dipe.org>

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
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGraphicsView>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaEnum>
#include <QtCore/QPointer>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>

// KDE
#include <KIcon>
#include <KIconLoader>
#include <KConfigDialog>
#include <KMenu>
#include <KProcess>
#include <KActionCollection>
#include <KBookmarkMenu>
#include <KRun>
#include <KServiceTypeTrader>
#include <KCModuleInfo>
#include <KToolInvocation>
#include <KIconButton>

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

    QPointer<Kickoff::MenuView> menuview;
    Plasma::IconWidget *icon;
    QPointer<Kickoff::UrlItemLauncher> launcher;

    KActionCollection* bookmarkcollection;
    BookmarkOwner* bookmarkowner;
    KBookmarkMenu* bookmarkmenu;

    QStringList viewtypes;
    MenuLauncherApplet::FormatType formattype;
    int maxRecentApps;

    QListWidget *view;
    KIconButton *iconButton;
    QComboBox *formatComboBox;
    QSpinBox *recentApplicationsSpinBox;

    QList<QAction*> actions;
    QAction* switcher;

    explicit Private(MenuLauncherApplet *q)
            : q(q),
            menuview(0), icon(0), launcher(0),
            bookmarkcollection(0), bookmarkowner(0), bookmarkmenu(0),
            view(0), iconButton(0), formatComboBox(0),
            switcher(0) {}
    ~Private() {
        delete bookmarkmenu;
        delete bookmarkowner;
        delete menuview;
    }

    void setMaxRecentApps(int num) {
        maxRecentApps = qMax(0, num);
        if (maxRecentApps > Kickoff::RecentApplications::self()->maximum()) {
            Kickoff::RecentApplications::self()->setMaximum(maxRecentApps);
        }
    }

    void addItem(QComboBox* combo, const QString& caption, int index, const QString& icon = QString()) {
        if (icon.isEmpty()) {
            combo->addItem(caption, index);
        } else {
            combo->addItem(KIcon(icon), caption, index);
        }
    }

    void setCurrentItem(QComboBox* combo, int currentIndex) {
        for (int i = combo->count() - 1; i >= 0; --i) {
            if (combo->itemData(i).toInt() == currentIndex) {
                combo->setCurrentIndex(i);
                return;
            }
        }
        if (combo->count() > 0)
            combo->setCurrentIndex(0);
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
            case RunCommand:               return i18n("Run Command");
            case SwitchUser:               return i18n("Switch User");
            case SaveSession:              return i18n("Save Session");
            case LockScreen:               return i18n("Lock Screen");
            case Logout:                   return i18n("Logout");
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
            case Logout:                   return "system-log-out";
            case Leave:                    return "system-shutdown";
        }
        return QString();
    }

    /*
    MenuLauncherApplet::ViewTypes viewTypes(const QByteArray& types) const {
        QMetaEnum e = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewType"));
        return (MenuLauncherApplet::ViewTypes) e.keyToValue(types);
    }
    QByteArray viewTypes(MenuLauncherApplet::ViewTypes types) const {
        QMetaEnum e = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewType"));
        return e.valueToKeys(types);
    }
    */

    void updateTooltip() {
        Plasma::ToolTipContent data(i18n("Classic Application Launcher"), i18n("Traditional menu based application launcher"), icon->icon());
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
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleMenu()));

    d->viewtypes << "RecentlyUsedApplications" << "Applications" << "Favorites" << "RunCommand" << "SwitchUser" << "SaveSession" << "LockScreen" << "Logout";
    d->formattype = NameDescription;

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(d->icon);
}

MenuLauncherApplet::~MenuLauncherApplet()
{
    delete d;
}

void MenuLauncherApplet::init()
{
    KConfigGroup cg = config();
    QString iconname = "start-here-kde";

    QStringList viewtypes = cg.readEntry("views", QStringList());
    if(viewtypes.isEmpty()) { // backward-compatibility to <KDE4.3
        QByteArray oldview = cg.readEntry("view", QByteArray());
        if(oldview == "Combined") {
            d->viewtypes = QStringList() << "Applications" << "Favorites" << "RunCommand" << "SwitchUser" << "SaveSession" << "LockScreen" << "Logout";
        } else if(! oldview.isEmpty()) {
            d->viewtypes = QStringList() << oldview;
            iconname = d->viewIcon( (MenuLauncherApplet::ViewType) metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType")).keyToValue(oldview) );
        }
    } else {
        d->viewtypes = viewtypes;
    }
    
    QMetaEnum fte = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
    QByteArray ftb = cg.readEntry("format", QByteArray(fte.valueToKey(d->formattype)));
    d->formattype = (MenuLauncherApplet::FormatType) fte.keyToValue(ftb);

    d->setMaxRecentApps(cg.readEntry("maxRecentApps", qMin(5, Kickoff::RecentApplications::self()->maximum())));

    d->icon->setIcon(KIcon(cg.readEntry("icon", iconname)));

    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ExtensionHandler, "desktop", new Kickoff::ServiceItemHandler);
    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ProtocolHandler, "leave", new Kickoff::LeaveItemHandler);

    if (KService::serviceByStorageId("kde4-kmenuedit.desktop")) {
        QAction* menueditor = new QAction(i18n("Menu Editor"), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT(! d->switcher);
    d->switcher = new QAction(i18n("Switch to Kickoff Menu Style"), this);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    Plasma::ToolTipManager::self()->registerWidget(this);
    d->updateTooltip();

    constraintsEvent(Plasma::ImmutableConstraint);
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
        containment()->addApplet("launcher", QVariantList(), geometry());
        destroy();
    }
}

void MenuLauncherApplet::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void MenuLauncherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *viewpage = new QWidget(parent);
    QVBoxLayout *l = new QVBoxLayout(viewpage);
    l->setMargin(0);
    viewpage->setLayout(l);
    d->view = new QListWidget(viewpage);
    d->view->resize(300,500);
    l->addWidget(d->view);
    foreach(ViewType vt, QList<ViewType>()<<RecentlyUsedApplications<<RecentlyUsedDocuments<<Applications<<Favorites<<Bookmarks<<Computer<<RecentlyUsed<<Settings<<RunCommand<<SwitchUser<<SaveSession<<LockScreen<<Logout<<Leave) {
        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
        QByteArray vtname = e.valueToKey(vt);

        QListWidgetItem *item = new QListWidgetItem(KIcon(d->viewIcon(vt)), d->viewText(vt), d->view);
        item->setCheckState(d->viewtypes.contains(vtname) ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, vtname);
        d->view->addItem(item);
    }
    parent->addPage(viewpage, i18n("View"), "view-list-details");

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
    d->formatComboBox = new QComboBox(p);
    formatLabel->setBuddy(d->formatComboBox);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Only"), MenuLauncherApplet::Name);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description Only"), MenuLauncherApplet::Description);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Description"), MenuLauncherApplet::NameDescription);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description (Name)"), MenuLauncherApplet::DescriptionName);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name - Description"), MenuLauncherApplet::NameDashDescription);
    d->setCurrentItem(d->formatComboBox, d->formattype);
    grid->addWidget(d->formatComboBox, 1, 1);

    QLabel *recentLabel = new QLabel(i18n("Recently Used Applications:"), p);
    grid->addWidget(recentLabel, 2, 0, Qt::AlignRight);
    d->recentApplicationsSpinBox = new QSpinBox(p);
    d->recentApplicationsSpinBox->setMaximum(10);
    d->recentApplicationsSpinBox->setMinimum(0);
    d->recentApplicationsSpinBox->setValue(d->maxRecentApps);
    recentLabel->setBuddy(d->recentApplicationsSpinBox);
    grid->addWidget(d->recentApplicationsSpinBox, 2, 1);

    grid->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0, 1, 3);
    parent->addPage(p, i18n("Options"), "configure");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void MenuLauncherApplet::configAccepted()
{
    bool needssaving = false;
    KConfigGroup cg = config();

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

    const int maxRecentApps = d->recentApplicationsSpinBox->value();
    if (maxRecentApps != d->maxRecentApps) {
        needssaving = true;
        d->setMaxRecentApps(maxRecentApps);
        cg.writeEntry("maxRecentApps", maxRecentApps);
    }

    if (needssaving) {
        d->updateTooltip();
        emit configNeedsSaving();
        if(d->menuview)
            d->menuview->deleteLater();
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

void MenuLauncherApplet::toggleMenu(bool pressed)
{
    if (! pressed) {
        return;
    }
    if (!d->menuview) {
        d->menuview = new Kickoff::MenuView();
        d->menuview->setFormatType( (Kickoff::MenuView::FormatType) d->formattype );
        connect(d->menuview, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
        connect(d->menuview, SIGNAL(aboutToHide()), d->icon, SLOT(setUnpressed()));
        //connect(d->menuview, SIGNAL(afterBeingHidden()), d->menuview, SLOT(deleteLater()));

        foreach(QString vtname, d->viewtypes) {
            if(vtname == "Applications") {
                if(d->menuview->actions().count() > 0)
                    d->menuview->addSeparator();
                Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(d->menuview);
                appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
                if (d->formattype == Name || d->formattype == NameDescription || d->formattype == NameDashDescription)
                    appModel->setPrimaryNamePolicy(Kickoff::ApplicationModel::AppNamePrimary);
                appModel->setSystemApplicationPolicy(Kickoff::ApplicationModel::ShowApplicationAndSystemPolicy);
                d->menuview->addModel(appModel, false);
            } else if(vtname == "Favorites") {
                if(d->menuview->actions().count() > 0)
                    d->menuview->addSeparator();
                d->menuview->addModel(new Kickoff::FavoritesModel(d->menuview), d->viewtypes.count() < 2);
            } else if(vtname == "Computer") {
                d->menuview->addModel(new Kickoff::SystemModel(d->menuview), d->viewtypes.count() < 2);
            } else if(vtname == "RecentlyUsed") {
                d->menuview->addModel(new Kickoff::RecentlyUsedModel(d->menuview), d->viewtypes.count() < 2);
            } else if(vtname == "RecentlyUsedApplications") {
                if(d->menuview->actions().count() > 0)
                    d->menuview->addSeparator();
                if (d->maxRecentApps > 0)
                    d->menuview->addModel(new Kickoff::RecentlyUsedModel(d->menuview, Kickoff::RecentlyUsedModel::ApplicationsOnly, d->maxRecentApps), true);
            } else if(vtname == "RecentlyUsedDocuments") {
                if(d->menuview->actions().count() > 0)
                    d->menuview->addSeparator();
                d->menuview->addModel(new Kickoff::RecentlyUsedModel(d->menuview, Kickoff::RecentlyUsedModel::DocumentsOnly), true);
            } else if(vtname == "Bookmarks") {
                KMenu* menu = d->menuview;
                if(d->viewtypes.count() >= 2) {
                    menu = new KMenu(d->viewText(Bookmarks), d->menuview);
                    menu->setIcon(KIcon(d->viewIcon(Bookmarks)));
                    d->menuview->addMenu(menu);
                }
                KBookmarkManager* mgr = KBookmarkManager::userBookmarksManager();
                if (! d->bookmarkcollection) {
                    d->bookmarkcollection = new KActionCollection(this);
                    d->bookmarkowner = new BookmarkOwner();
                }
                delete d->bookmarkmenu;
                d->bookmarkmenu = new KBookmarkMenu(mgr, d->bookmarkowner, menu, d->bookmarkcollection);
            } else if(vtname == "Settings") {
                KMenu* parentmenu = d->menuview;
                if(d->viewtypes.count() >= 2) {
                    parentmenu = new KMenu(d->viewText(Settings), d->menuview);
                    parentmenu->setIcon(KIcon(d->viewIcon(Settings)));
                    d->menuview->addMenu(parentmenu);
                }
                QMap<QString, KMenu*> menus;
                foreach(KService::Ptr rootentry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", "(not exist [X-KDE-System-Settings-Parent-Category]) or [X-KDE-System-Settings-Parent-Category]==''"))) {
                    parentmenu->addTitle(rootentry->name().replace('&',"&&"));
                    const QString rootcategory = rootentry->property("X-KDE-System-Settings-Category").toString();
                    foreach(KService::Ptr entry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", QString("[X-KDE-System-Settings-Parent-Category]=='%1'").arg(rootcategory)))) {
                        KMenu* menu = new KMenu(entry->name().replace('&',"&&"), parentmenu);
                        menu->setIcon(KIcon(entry->icon()));
                        parentmenu->addMenu(menu);
                        const QString category = entry->property("X-KDE-System-Settings-Category").toString();
                        menus[category] = menu;
                    }
                }
                QMap<QString, QList<KService::Ptr> > modules;
                foreach(KService::Ptr entry, sortServices(KServiceTypeTrader::self()->query("KCModule"))) {
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
                    foreach(KService::Ptr entry, modules[category]) {
                        KCModuleInfo module(entry->entryPath());
                        QAction* a = m->addAction(KIcon(module.icon()), module.moduleName().replace('&',"&&"));
                        a->setData(KUrl("kcm:/" + entry->entryPath()));
                    }
                }
            } else if(vtname == "RunCommand") {
                d->menuview->addAction(KIcon(d->viewIcon(RunCommand)), d->viewText(RunCommand))->setData(KUrl("leave:/run"));
            } else if(vtname == "SwitchUser") {
                d->menuview->addAction(KIcon(d->viewIcon(SwitchUser)), d->viewText(SwitchUser))->setData(KUrl("leave:/switch"));
            } else if(vtname == "SaveSession") {
                KConfigGroup c(KSharedConfig::openConfig("ksmserverrc", KConfig::NoGlobals), "General");
                if (c.readEntry("loginMode") == "restoreSavedSession") {
                    d->menuview->addAction(KIcon(d->viewIcon(SaveSession)), d->viewText(SaveSession))->setData(KUrl("leave:/savesession"));
                }
            } else if(vtname == "LockScreen") {
                d->menuview->addAction(KIcon(d->viewIcon(LockScreen)), d->viewText(LockScreen))->setData(KUrl("leave:/lock"));
            } else if(vtname == "Logout") {
                d->menuview->addAction(KIcon(d->viewIcon(Logout)), d->viewText(Logout))->setData(KUrl("leave:/logout"));
            } else if(vtname == "Leave") {
                if(d->menuview->actions().count() > 0)
                    d->menuview->addSeparator();
                Kickoff::LeaveModel *leavemodel = new Kickoff::LeaveModel(d->menuview);
                leavemodel->updateModel();
                d->menuview->addModel(leavemodel, true);
            }
        }
    }

	Plasma::ToolTipManager::self()->hide(this);
    d->menuview->setAttribute(Qt::WA_DeleteOnClose);
    d->menuview->popup(popupPosition(d->menuview->sizeHint()));
    d->icon->setPressed();
}

void MenuLauncherApplet::actionTriggered(QAction *action)
{
    KUrl url = action->data().value<KUrl>();
    if (url.scheme() == "leave") {
        if (! d->launcher)
            d->launcher = new Kickoff::UrlItemLauncher(d->menuview);
        d->launcher->openUrl(url.url());
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


#include "simpleapplet.moc"
