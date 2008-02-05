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

// KDE
#include <KIcon>
#include <KDialog>
#include <KMenu>

// Plasma
#include <plasma/layouts/boxlayout.h>
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
        KMenu *menuview;
        Plasma::Icon *icon;
        QPointer<Kickoff::UrlItemLauncher> launcher;

        MenuLauncherApplet::ViewType viewtype;
        MenuLauncherApplet::FormatType formattype;

        KDialog *dialog;
        QComboBox *viewComboBox, *formatComboBox;

        Private() : menuview(0), launcher(0), dialog(0) {}
        ~Private() { delete dialog; delete menuview; }

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
                model->setParent(view); //re-parent
                view->setModel(model);
            }
            return view;
        }

        void addMenu(Kickoff::MenuView *view, bool mergeFirstLevel) {
            QList<QAction*> actions = view->actions();
            foreach(QAction *action, actions) {
                if( action->menu() && mergeFirstLevel ) {
                    QMetaObject::invokeMethod(action->menu(),"aboutToShow"); //fetch the children
                    QList<QAction*> subactions;
                    foreach(QAction *a, action->menu()->actions()) {
                        if( a->menu() || ! view->indexForAction(a).data(Kickoff::UrlRole).isNull() ) {
                            subactions << a;
                        }
                    }
                    if( actions.count() > 1 && subactions.count() > 0 ) {
                        menuview->addTitle(action->text());
                    }
                    foreach(QAction *a, subactions) {
                        menuview->addAction(a);
                    }
                }
                else {
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
                    return "view-history";
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

    d->icon = new Plasma::Icon(QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));

    d->viewtype = Combined;
    d->formattype = NameDescription;
}

MenuLauncherApplet::~MenuLauncherApplet()
{
    delete d;
}

void MenuLauncherApplet::init()
{
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

    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ExtensionHandler,"desktop",new Kickoff::ServiceItemHandler);
    Kickoff::UrlItemLauncher::addGlobalHandler(Kickoff::UrlItemLauncher::ProtocolHandler, "leave", new Kickoff::LeaveItemHandler);
}

void MenuLauncherApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            setMinimumContentSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            setMinimumContentSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        d->icon->resize(contentSize());
    }
}

void MenuLauncherApplet::showConfigurationInterface()
{
    if (! d->dialog) {
        d->dialog = new KDialog();
        d->dialog->setCaption( i18nc("@title:window","Configure Menu") );
        d->dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect(d->dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(d->dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        QWidget *p = d->dialog->mainWidget();
        QGridLayout *l = new QGridLayout(p);
        p->setLayout(l);

        QLabel *viewLabel = new QLabel(i18n("View:"), p);
        l->addWidget(viewLabel, 0, 0);
        d->viewComboBox = new QComboBox(p);
        viewLabel->setBuddy(d->viewComboBox);
        d->addItem(d->viewComboBox, i18n("Standard"), MenuLauncherApplet::Combined, "start-here-kde");
        d->addItem(d->viewComboBox, i18n("Favorites"), MenuLauncherApplet::Favorites, "bookmarks");
        d->addItem(d->viewComboBox, i18n("Applications"), MenuLauncherApplet::Applications, "applications-other");
        d->addItem(d->viewComboBox, i18n("Computer"), MenuLauncherApplet::Computer, "computer");
        d->addItem(d->viewComboBox, i18n("Recently Used"), MenuLauncherApplet::RecentlyUsed, "view-history");
        d->addItem(d->viewComboBox, i18n("Leave"), MenuLauncherApplet::Leave, "application-exit");
        l->addWidget(d->viewComboBox, 0, 1);

        QLabel *formatLabel = new QLabel(i18n("Format:"), p);
        l->addWidget(formatLabel, 1, 0);
        d->formatComboBox = new QComboBox(p);
        formatLabel->setBuddy(d->formatComboBox);
        d->addItem(d->formatComboBox, i18n("Name only"), MenuLauncherApplet::Name);
        d->addItem(d->formatComboBox, i18n("Description only"), MenuLauncherApplet::Description);
        d->addItem(d->formatComboBox, i18n("Name Description"), MenuLauncherApplet::NameDescription);
        d->addItem(d->formatComboBox, i18n("Description (Name)"), MenuLauncherApplet::DescriptionName);
        l->addWidget(d->formatComboBox, 1, 1);

        l->setColumnStretch(1,1);
    }

    d->setCurrentItem(d->viewComboBox, d->viewtype);
    d->setCurrentItem(d->formatComboBox, d->formattype);
    d->dialog->show();
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

Qt::Orientations MenuLauncherApplet::expandingDirections() const
{
    return 0;
}

void MenuLauncherApplet::toggleMenu(bool pressed)
{
    if (!pressed) {
        return;
    }

    if (!d->menuview) {
        d->menuview = new KMenu();
        connect(d->menuview,SIGNAL(triggered(QAction*)),this,SLOT(actionTriggered(QAction*)));
        connect(d->menuview,SIGNAL(aboutToHide()),d->icon,SLOT(setUnpressed()));

        switch( d->viewtype ) {
            case Combined: {
                ApplicationModel *appModel = new ApplicationModel();
                appModel->setDuplicatePolicy(ApplicationModel::ShowLatestOnlyPolicy);
                Kickoff::MenuView *appview = d->createMenuView(appModel);
                d->addMenu(appview, false);

                d->menuview->addSeparator();
                Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
                d->addMenu(favview, false);

                d->menuview->addSeparator();
                QAction *switchaction = d->menuview->addAction(KIcon("system-switch-user"),i18n("Switch User"));
                switchaction->setData(QUrl("leave:/switch"));
                QAction *lockaction = d->menuview->addAction(KIcon("system-lock-screen"),i18n("Lock"));
                lockaction->setData(QUrl("leave:/lock"));
                QAction *logoutaction = d->menuview->addAction(KIcon("system-log-out"),i18n("Logout"));
                logoutaction->setData(QUrl("leave:/logout"));
            } break;
            case Favorites: {
                Kickoff::MenuView *favview = d->createMenuView(new Kickoff::FavoritesModel(d->menuview));
                d->addMenu(favview, true);
            } break;
            case Applications: {
                ApplicationModel *appModel = new ApplicationModel();
                appModel->setDuplicatePolicy(ApplicationModel::ShowLatestOnlyPolicy);
                Kickoff::MenuView *appview = d->createMenuView(appModel);
                d->addMenu(appview, false);
            } break;
            case Computer: {
                Kickoff::MenuView *systemview = d->createMenuView(new Kickoff::SystemModel());
                d->addMenu(systemview, true);
            } break;
            case RecentlyUsed: {
                Kickoff::MenuView *recentlyview = d->createMenuView(new Kickoff::RecentlyUsedModel());
                d->addMenu(recentlyview, true);
            } break;
            case Leave: {
                Kickoff::MenuView *leaveview = d->createMenuView(new Kickoff::LeaveModel(d->menuview));
                d->addMenu(leaveview, true);
            } break;
        }
    }

    d->menuview->setAttribute(Qt::WA_DeleteOnClose);
    d->menuview->popup(d->icon->popupPosition(d->menuview->sizeHint()));
    d->icon->setPressed();
}

void MenuLauncherApplet::actionTriggered(QAction *action)
{
    if (action->data().type() == QVariant::Url) {
        KUrl url = action->data().toUrl();
        if (url.scheme() == "leave") {
            if ( ! d->launcher ) {
                d->launcher = new Kickoff::UrlItemLauncher(d->menuview);
            }
            d->launcher->openUrl(url.url());
        }
    }
    else {
        for(QWidget* w = action->parentWidget(); w; w = w->parentWidget()) {
            if (Kickoff::MenuView *view = dynamic_cast<Kickoff::MenuView*>(w)) {
                view->actionTriggered(action);
                break;
            }
        }
    }
}

#include "simpleapplet.moc"
