/*
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
#include "simpleapplet/simpleapplet.h"
#include "simpleapplet/menuview.h"

// Qt
#include <QLabel>
#include <QComboBox>
#include <QGridLayout>
#include <QGraphicsView>
#include <QMetaObject>
#include <QMetaEnum>
#include <QPointer>
#include <QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <KConfigDialog>
#include <KMenu>
#include <KProcess>

// Plasma
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

// Local
#include "core/itemhandlers.h"
#include "core/models.h"
#include "core/applicationmodel.h"
#include "core/favoritesmodel.h"
#include "core/systemmodel.h"
#include "core/recentlyusedmodel.h"
#include "core/leavemodel.h"
#include "core/urlitemlauncher.h"

class MenuLauncherApplet::Private
{
public:
        Kickoff::MenuView *menuview;
        Plasma::Icon *icon;
        QPointer<Kickoff::UrlItemLauncher> launcher;

        MenuLauncherApplet::ViewType viewtype;
        MenuLauncherApplet::FormatType formattype;

        QComboBox *viewComboBox, *formatComboBox;

        QList<QAction*> actions;
        QAction* switcher;

        Private() : menuview(0), launcher(0), switcher(0) {}
        ~Private() { delete menuview; }

        void addItem(QComboBox* combo, const QString& caption, int index, const QString& icon = QString()) {
            if( icon.isNull() ) {
                combo->addItem(caption, index);
            }
            else {
                combo->addItem(KIcon(icon), caption, index);
            }
        }

        void setCurrentItem(QComboBox* combo, int currentIndex) {
            for(int i = combo->count() - 1; i >= 0; --i) {
                if( combo->itemData(i).toInt() == currentIndex ) {
                    combo->setCurrentIndex(i);
                    return;
                }
            }
            if( combo->count() > 0 ) {
                combo->setCurrentIndex(0);
            }
        }

        Kickoff::MenuView *createMenuView(QAbstractItemModel *model = 0) {
            Kickoff::MenuView *view = new Kickoff::MenuView(menuview);
            view->setFormatType( (Kickoff::MenuView::FormatType) formattype );
            if( model ) {
                view->setModel(model);
            }
            return view;
        }

        void addMenu(Kickoff::MenuView *view, bool mergeFirstLevel) {
            QList<QAction*> actions = view->actions();
            foreach(QAction *action, actions) {
                if( action->menu() && mergeFirstLevel ) {
                    QMetaObject::invokeMethod(action->menu(),"aboutToShow"); //fetch the children
                    if( actions.count() > 1 && action->menu()->actions().count() > 0 ) {
                        menuview->addTitle(action->text());
                    }
                    foreach(QAction *a, action->menu()->actions()) {
                        a->setVisible(a->menu() || ! view->indexForAction(a).data(Kickoff::UrlRole).isNull());
                        menuview->addAction(a);
                    }
                }
                else {
                    action->setVisible(action->menu() || ! view->indexForAction(action).data(Kickoff::UrlRole).isNull());
                    menuview->addAction(action);
                }
            }
        }

        QString viewIcon() {
            switch( viewtype ) {
                case Combined:
                    return "start-here-kde";
                case Favorites:
                    return "bookmarks";
                case Applications:
                    return "applications-other";
                case Computer:
                    return "computer";
                case RecentlyUsed:
                    return "document-open-recent";
                case Leave:
                    return "application-exit";
            }
            return QString();
        }

};

MenuLauncherApplet::MenuLauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      d(new Private)
{
    setHasConfigurationInterface(true);
    setBackgroundHints(NoBackground);

    d->icon = new Plasma::Icon(QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleMenu()));

    d->viewtype = Combined;
    d->formattype = NameDescription;

    resize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
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

    {
        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
        QByteArray ba = cg.readEntry("view", QByteArray(e.valueToKey(d->viewtype)));
        d->viewtype = (MenuLauncherApplet::ViewType) e.keyToValue(ba);
    }
    {
        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
        QByteArray ba = cg.readEntry("format", QByteArray(e.valueToKey(d->formattype)));
        d->formattype = (MenuLauncherApplet::FormatType) e.keyToValue(ba);
    }

    d->icon->setIcon(KIcon(d->viewIcon()));
    //d->icon->setIcon(KIcon(cg.readEntry("icon","start-here-kde")));
    //setMinimumContentSize(d->icon->iconSize()); //setSize(d->icon->iconSize())

    setAspectRatioMode(Plasma::ConstrainedSquare);

    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ExtensionHandler,"desktop",new Kickoff::ServiceItemHandler);
    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ProtocolHandler, "leave", new Kickoff::LeaveItemHandler);

    if (KService::serviceByStorageId("kde4-kmenuedit.desktop")) {
        QAction* menueditor = new QAction(i18n("Menu Editor"), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT( ! d->switcher );
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

    if (constraints & Plasma::ImmutableConstraint) {
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
    QWidget *p = new QWidget;
    QGridLayout *l = new QGridLayout(p);
    p->setLayout(l);

    QLabel *viewLabel = new QLabel(i18nc("@label:listbox Which category of items to view in a KMenu-like menu", "View:"), p);
    l->addWidget(viewLabel, 0, 0);
    d->viewComboBox = new QComboBox(p);
    viewLabel->setBuddy(d->viewComboBox);
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Standard"), MenuLauncherApplet::Combined, "start-here-kde");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Favorites"), MenuLauncherApplet::Favorites, "bookmarks");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Applications"), MenuLauncherApplet::Applications, "applications-other");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Computer"), MenuLauncherApplet::Computer, "computer");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Recently Used"), MenuLauncherApplet::RecentlyUsed, "document-open-recent");
    d->addItem(d->viewComboBox, i18nc("@item:inlistbox View:", "Leave"), MenuLauncherApplet::Leave, "application-exit");
    l->addWidget(d->viewComboBox, 0, 1);

    QLabel *formatLabel = new QLabel(i18nc("@label:listbox How to present applications in a KMenu-like menu", "Format:"), p);
    l->addWidget(formatLabel, 1, 0);
    d->formatComboBox = new QComboBox(p);
    formatLabel->setBuddy(d->formatComboBox);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Only"), MenuLauncherApplet::Name);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description Only"), MenuLauncherApplet::Description);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Name Description"), MenuLauncherApplet::NameDescription);
    d->addItem(d->formatComboBox, i18nc("@item:inlistbox Format:", "Description (Name)"), MenuLauncherApplet::DescriptionName);
    l->addWidget(d->formatComboBox, 1, 1);

    l->setColumnStretch(1,1);

    d->setCurrentItem(d->viewComboBox, d->viewtype);
    d->setCurrentItem(d->formatComboBox, d->formattype);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(p, parent->windowTitle(), icon());
}

void MenuLauncherApplet::configAccepted()
{
    bool needssaving = false;
    KConfigGroup cg = config();

    int vt = d->viewComboBox->itemData(d->viewComboBox->currentIndex()).toInt();
    if( vt != d->viewtype ) {
        d->viewtype = (MenuLauncherApplet::ViewType) vt;
        needssaving = true;

        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("ViewType"));
        cg.writeEntry("view", QByteArray(e.valueToKey(d->viewtype)));

        d->icon->setIcon(KIcon(d->viewIcon()));
        d->icon->update();
    }

    int ft = d->formatComboBox->itemData(d->formatComboBox->currentIndex()).toInt();
    if( ft != d->formattype ) {
        d->formattype = (MenuLauncherApplet::FormatType) ft;
        needssaving = true;

        QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("FormatType"));
        cg.writeEntry("format", QByteArray(e.valueToKey(d->formattype)));
    }

    if( needssaving ) {
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

void MenuLauncherApplet::toggleMenu()
{
    if (!d->menuview) {
        d->menuview = new Kickoff::MenuView();
        connect(d->menuview, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
        connect(d->menuview, SIGNAL(aboutToHide()), d->icon, SLOT(setUnpressed()));
        connect(d->menuview, SIGNAL(destroyed(QObject*)), this, SLOT(menuDestroyed()));

        switch( d->viewtype ) {
            case Combined: {
                Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(d->menuview);
                appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
                Kickoff::MenuView *appview = d->createMenuView(appModel);
                d->addMenu(appview, false);

                d->menuview->addSeparator();
                Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
                d->addMenu(favview, false);

                d->menuview->addSeparator();
                QAction *switchaction = d->menuview->addAction(KIcon("system-switch-user"),i18n("Switch User"));
                switchaction->setData(KUrl("leave:/switch"));
                QAction *lockaction = d->menuview->addAction(KIcon("system-lock-screen"),i18n("Lock"));
                lockaction->setData(KUrl("leave:/lock"));
                QAction *logoutaction = d->menuview->addAction(KIcon("system-shutdown"),i18n("Leave"));
                logoutaction->setData(KUrl("leave:/logout"));
            } break;
            case Favorites: {
                Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
                d->addMenu(favview, true);
            } break;
            case Applications: {
                Kickoff::ApplicationModel *appModel = new Kickoff::ApplicationModel(d->menuview);
                appModel->setDuplicatePolicy(Kickoff::ApplicationModel::ShowLatestOnlyPolicy);
                Kickoff::MenuView *appview = d->createMenuView(appModel);
                d->addMenu(appview, false);
            } break;
            case Computer: {
                Kickoff::MenuView *systemview = d->createMenuView(new Kickoff::SystemModel(d->menuview));
                d->addMenu(systemview, true);
            } break;
            case RecentlyUsed: {
                Kickoff::MenuView *recentlyview = d->createMenuView(new Kickoff::RecentlyUsedModel(d->menuview));
                d->addMenu(recentlyview, true);
            } break;
            case Leave: {
                Kickoff::MenuView *leaveview = d->createMenuView(new Kickoff::LeaveModel(d->menuview));
                d->addMenu(leaveview, true);
            } break;
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
        if ( ! d->launcher ) {
            d->launcher = new Kickoff::UrlItemLauncher(d->menuview);
        }
        d->launcher->openUrl(url.url());
        return;
    }
    for(QWidget* w = action->parentWidget(); w; w = w->parentWidget()) {
        if (Kickoff::MenuView *view = dynamic_cast<Kickoff::MenuView*>(w)) {
            view->actionTriggered(action);
            break;
        }
    }
}

void MenuLauncherApplet::menuDestroyed()
{
    d->menuview = 0;
}

QList<QAction*> MenuLauncherApplet::contextualActions()
{
    return d->actions;
}

#include "simpleapplet.moc"
