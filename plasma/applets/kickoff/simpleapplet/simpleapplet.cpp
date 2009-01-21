/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>
    Copyright 2008 Sebastian Sauer <mail@dipe.org>

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

// KDE
#include <KIcon>
#include <KConfigDialog>
#include <KMenu>
#include <KProcess>
#include <KActionCollection>
#include <KBookmarkMenu>
#include <KRun>
#include <KServiceTypeTrader>
#include <KCModuleInfo>
#include <KCMultiDialog>

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
    QPointer<Kickoff::MenuView> menuview;
    Plasma::IconWidget *icon;
    QPointer<Kickoff::UrlItemLauncher> launcher;

    KActionCollection* collection;
    BookmarkOwner* bookmarkowner;
    KBookmarkMenu* bookmarkmenu;

    MenuLauncherApplet::ViewType viewtype;
    MenuLauncherApplet::FormatType formattype;
    int maxRecentApps;

    QComboBox *viewComboBox;
    QComboBox *formatComboBox;
    QSpinBox *recentApplicationsSpinBox;

    QList<QAction*> actions;
    QAction* switcher;

    Private()
            : menuview(0),
            icon(0),
            launcher(0),
            collection(0),
            bookmarkowner(0),
            bookmarkmenu(0),
            viewComboBox(0),
            formatComboBox(0),
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
        if (combo->count() > 0) {
            combo->setCurrentIndex(0);
        }
    }

    Kickoff::MenuView *createMenuView(QAbstractItemModel *model = 0) {
        Kickoff::MenuView *view = new Kickoff::MenuView(menuview);
        view->setFormatType((Kickoff::MenuView::FormatType) formattype);
        if (model) {
            view->setModel(model);
        }
        return view;
    }

    void addMenu(Kickoff::MenuView *view, bool mergeFirstLevel) {
        QList<QAction*> actions = view->actions();
        foreach(QAction *action, actions) {
            if (action->menu() && mergeFirstLevel) {
                QMetaObject::invokeMethod(action->menu(), "aboutToShow"); //fetch the children
                if (actions.count() > 1 && action->menu()->actions().count() > 0) {
                    menuview->addTitle(action->text());
                }
                foreach(QAction *a, action->menu()->actions()) {
                    a->setVisible(a->menu() || ! view->indexForAction(a).data(Kickoff::UrlRole).isNull());
                    menuview->addAction(a);
                }
            } else {
                action->setVisible(action->menu() || ! view->indexForAction(action).data(Kickoff::UrlRole).isNull());
                menuview->addAction(action);
            }
        }

        // if the model asks us for a reset we can't do much except to invalidate our
        // menuview to be able to rebuild it what is needed to prevent dealing with
        // invalid items.
        // the problem here is, that if the menu is currently displayed, it will just
        // close itself what is evil++ but still better than crashes. anyway, the
        // right(TM) solution would be to introduce logic to update the content of the
        // menu even on a reset.
        connect(view->model(), SIGNAL(modelReset()), menuview, SLOT(deleteLater()));
    }

    QString viewIcon() {
        switch (viewtype) {
        case Combined:
            return "start-here-kde";
        case Favorites:
            return "bookmarks";
        case Bookmarks:
            return "folder-bookmarks";
        case Applications:
            return "applications-other";
        case Computer:
            return "computer";
        case RecentlyUsed:
            return "document-open-recent";
        case Settings:
            return "preferences-system";
        case Leave:
            return "application-exit";
        }
        return QString();
    }

};

MenuLauncherApplet::MenuLauncherApplet(QObject *parent, const QVariantList &args)
        : Plasma::Applet(parent, args),
        d(new Private)
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");

    setHasConfigurationInterface(true);
    setBackgroundHints(NoBackground);

    resize(IconSize(KIconLoader::Desktop) * 2, IconSize(KIconLoader::Desktop) * 2);

    d->icon = new Plasma::IconWidget(QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleMenu()));

    d->viewtype = Combined;
    d->formattype = NameDescription;
}

MenuLauncherApplet::~MenuLauncherApplet()
{
    delete d;
}

void MenuLauncherApplet::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addItem(d->icon);

    KConfigGroup cg = config();

    QMetaEnum vte = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
    QByteArray vtb = cg.readEntry("view", QByteArray(vte.valueToKey(d->viewtype)));
    d->viewtype = (MenuLauncherApplet::ViewType) vte.keyToValue(vtb);

    QMetaEnum fte = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
    QByteArray ftb = cg.readEntry("format", QByteArray(fte.valueToKey(d->formattype)));
    d->formattype = (MenuLauncherApplet::FormatType) fte.keyToValue(ftb);

    d->setMaxRecentApps(cg.readEntry("maxRecentApps", qMin(5, Kickoff::RecentApplications::self()->maximum())));

    d->icon->setIcon(KIcon(d->viewIcon()));
    //d->icon->setIcon(KIcon(cg.readEntry("icon","start-here-kde")));
    //setMinimumContentSize(d->icon->iconSize()); //setSize(d->icon->iconSize())

    setAspectRatioMode(Plasma::ConstrainedSquare);

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

    constraintsEvent(Plasma::ImmutableConstraint);
}

void MenuLauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
                formFactor() == Plasma::MediaCenter) {
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
    QWidget *p = new QWidget(parent);
    QGridLayout *l = new QGridLayout(p);
    p->setLayout(l);

    QLabel *viewLabel = new QLabel(i18nc("@label:listbox Which category of items to view in a KMenu-like menu", "View:"), p);
    l->addWidget(viewLabel, 0, 0, Qt::AlignRight);
    d->viewComboBox = new QComboBox(p);
    viewLabel->setBuddy(d->viewComboBox);
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Standard"), MenuLauncherApplet::Combined, "start-here-kde");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Favorites"), MenuLauncherApplet::Favorites, "bookmarks");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Bookmarks"), MenuLauncherApplet::Bookmarks, "folder-bookmarks");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Applications"), MenuLauncherApplet::Applications, "applications-other");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Computer"), MenuLauncherApplet::Computer, "computer");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Recently Used"), MenuLauncherApplet::RecentlyUsed, "document-open-recent");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "System Settings"), MenuLauncherApplet::Settings, "preferences-system");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Leave"), MenuLauncherApplet::Leave, "application-exit");
    l->addWidget(d->viewComboBox, 0, 1);

    QLabel *formatLabel = new QLabel(i18nc("@label:listbox How to present applications in a KMenu-like menu", "Format:"), p);
    l->addWidget(formatLabel, 1, 0, Qt::AlignRight);
    d->formatComboBox = new QComboBox(p);
    formatLabel->setBuddy(d->formatComboBox);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Only"), MenuLauncherApplet::Name);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description Only"), MenuLauncherApplet::Description);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Description"), MenuLauncherApplet::NameDescription);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description (Name)"), MenuLauncherApplet::DescriptionName);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name - Description"), MenuLauncherApplet::NameDashDescription);
    l->addWidget(d->formatComboBox, 1, 1);

    QLabel *recentLabel = new QLabel(i18n("Recently Used Applications:"), p);
    l->addWidget(recentLabel, 2, 0, Qt::AlignRight);
    d->recentApplicationsSpinBox = new QSpinBox(p);
    d->recentApplicationsSpinBox->setMaximum(10);
    d->recentApplicationsSpinBox->setMinimum(0);
    d->recentApplicationsSpinBox->setValue(d->maxRecentApps);
    recentLabel->setBuddy(d->recentApplicationsSpinBox);
    l->addWidget(d->recentApplicationsSpinBox, 2, 1);

    l->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0, 1, 2);
    l->setColumnStretch(1, 1);

    d->setCurrentItem(d->viewComboBox, d->viewtype);
    d->setCurrentItem(d->formatComboBox, d->formattype);

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(p, i18n("General"), icon());
}

void MenuLauncherApplet::configAccepted()
{
    bool needssaving = false;
    KConfigGroup cg = config();

    const int vt = d->viewComboBox->itemData(d->viewComboBox->currentIndex()).toInt();
    if (vt != d->viewtype) {
        d->viewtype = (MenuLauncherApplet::ViewType) vt;
        needssaving = true;

        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
        cg.writeEntry("view", QByteArray(e.valueToKey(d->viewtype)));

        d->icon->setIcon(KIcon(d->viewIcon()));
        d->icon->update();
    }

    const int ft = d->formatComboBox->itemData(d->formatComboBox->currentIndex()).toInt();
    if (ft != d->formattype) {
        d->formattype = (MenuLauncherApplet::FormatType) ft;
        needssaving = true;

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
        emit configNeedsSaving();

        delete d->menuview;
        d->menuview = 0;
    }
}

void MenuLauncherApplet::toggleMenu(bool pressed)
{
    if (pressed) {
        toggleMenu();
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
    if (!d->menuview) {
        d->menuview = new Kickoff::MenuView();
        connect(d->menuview, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
        connect(d->menuview, SIGNAL(aboutToHide()), d->icon, SLOT(setUnpressed()));
        connect(d->menuview, SIGNAL(aboutToHide()), d->menuview, SLOT(deleteLater()));

        switch (d->viewtype) {
        case Combined: {
            if (d->maxRecentApps > 0) {
                d->menuview->addTitle(i18n("Recently Used Applications"));
                Kickoff::RecentlyUsedModel* recentlymodel = new Kickoff::RecentlyUsedModel(d->menuview, Kickoff::RecentlyUsedModel::ApplicationsOnly, d->maxRecentApps);
                Kickoff::MenuView *recentlyview = d->createMenuView(recentlymodel);
                d->addMenu(recentlyview, true);
            }

            d->menuview->addTitle(i18n("All Applications"));
            Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(d->menuview);
            appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
            appModel->setSystemApplicationPolicy(Kickoff::ApplicationModel::ShowApplicationAndSystemPolicy);
            Kickoff::MenuView *appview = d->createMenuView(appModel);
            d->addMenu(appview, false);

            d->menuview->addSeparator();
            Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
            d->addMenu(favview, false);

            d->menuview->addTitle(i18n("Actions"));
            QAction *runaction = d->menuview->addAction(KIcon("system-run"), i18n("Run Command..."));
            runaction->setData(KUrl("leave:/run"));
            d->menuview->addSeparator();
            QAction *switchaction = d->menuview->addAction(KIcon("system-switch-user"), i18n("Switch User"));
            switchaction->setData(KUrl("leave:/switch"));
            QAction *lockaction = d->menuview->addAction(KIcon("system-lock-screen"), i18n("Lock Screen"));
            lockaction->setData(KUrl("leave:/lock"));
            QAction *logoutaction = d->menuview->addAction(KIcon("system-shutdown"), i18n("Leave..."));
            logoutaction->setData(KUrl("leave:/logout"));
        }
        break;
        case Favorites: {
            Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
            d->addMenu(favview, true);
        }
        break;
        case Applications: {
            Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(d->menuview);
            appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
            Kickoff::MenuView *appview = d->createMenuView(appModel);
            d->addMenu(appview, false);
        }
        break;
        case Computer: {
            Kickoff::MenuView *systemview = d->createMenuView(new Kickoff::SystemModel(d->menuview));
            d->addMenu(systemview, true);
        }
        break;
        case RecentlyUsed: {
            Kickoff::MenuView *recentlyview = d->createMenuView(new Kickoff::RecentlyUsedModel(d->menuview));
            d->addMenu(recentlyview, true);
        }
        break;
        case Bookmarks: {
            KBookmarkManager* mgr = KBookmarkManager::userBookmarksManager();
            if (! d->collection) {
                d->collection = new KActionCollection(this);
                d->bookmarkowner = new BookmarkOwner();
            }
            delete d->bookmarkmenu;
            d->bookmarkmenu = new KBookmarkMenu(mgr, d->bookmarkowner, d->menuview, d->collection);
        }
        break;
        case Settings: {
            QMap<QString, KMenu*> menus;
            foreach(KService::Ptr rootentry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", "(not exist [X-KDE-System-Settings-Parent-Category]) or [X-KDE-System-Settings-Parent-Category]==''"))) {
                d->menuview->addTitle(rootentry->name().replace('&',"&&"));
                const QString rootcategory = rootentry->property("X-KDE-System-Settings-Category").toString();
                foreach(KService::Ptr entry, sortServices(KServiceTypeTrader::self()->query("SystemSettingsCategory", QString("[X-KDE-System-Settings-Parent-Category]=='%1'").arg(rootcategory)))) {
                    KMenu* menu = new KMenu(entry->name().replace('&',"&&"), d->menuview);
                    menu->setIcon(KIcon(entry->icon()));
                    d->menuview->addMenu(menu);
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
                    Q_ASSERT(services.count() > 0); //if that happens then we miss the desktop-file defining the category
                    Q_ASSERT(services.count() < 2); //if that happens then we have more then one desktop-file defining the same category
                    if(services.count() < 1) {
                        menucategory.clear();
                    } else {
                        subcategory = services[0];
                        menucategory = subcategory->property("X-KDE-System-Settings-Parent-Category").toString();
                    }
                }
                if(menucategory.isEmpty()) continue; //skip the category
                KMenu* menu = menus[menucategory];
                if(! subcategory.isNull())
                    menu->addTitle(subcategory->name());
                foreach(KService::Ptr entry, modules[category]) {
                    KCModuleInfo module(entry->entryPath());
                    QAction* a = menu->addAction(KIcon(module.icon()), module.moduleName().replace('&',"&&"));
                    a->setData(KUrl("kcm:/" + entry->entryPath()));
                }
            }
        }
        break;
        case Leave: {
            Kickoff::MenuView *leaveview = d->createMenuView(new Kickoff::LeaveModel(d->menuview));
            d->addMenu(leaveview, true);
        }
        break;
        }
    }

    d->menuview->setAttribute(Qt::WA_DeleteOnClose);
    d->menuview->popup(popupPosition(d->menuview->sizeHint()));
    d->icon->setPressed();
}

void MenuLauncherApplet::actionTriggered(QAction *action)
{
    KUrl url = action->data().value<KUrl>();
    if (url.scheme() == "leave") {
        if (! d->launcher) {
            d->launcher = new Kickoff::UrlItemLauncher(d->menuview);
        }
        d->launcher->openUrl(url.url());
        return;
    }
    if (url.scheme() == "kcm") {
        KCMultiDialog *dialog = new KCMultiDialog();
        dialog->addModule(url.fileName());
        dialog->show();
        connect(dialog, SIGNAL(finished(int)), dialog, SLOT(delayedDestruct()));
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
