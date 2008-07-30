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
#include "applet/applet.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <KDebug>
#include <KConfigDialog>
#include <KNumInput>
#include <KProcess>

// Plasma
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>
#include <plasma/view.h>

// Local
#include "ui/launcher.h"

class LauncherApplet::Private
{
public:
    Plasma::Icon *icon;
    Kickoff::Launcher *launcher;

    QCheckBox *switchOnHoverCheckBox;
    KIntNumInput *visibleCountEdit;
    QList<QAction*> actions;
    QAction* switcher;

    Private() : launcher(0), switcher(0) {}
    ~Private() { delete launcher; }
    void createLauncher(LauncherApplet *q);
};

void LauncherApplet::Private::createLauncher(LauncherApplet *q)
{
    launcher = new Kickoff::Launcher(q);
    launcher->setWindowFlags(launcher->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
    launcher->setAutoHide(true);
    launcher->adjustSize();
    QObject::connect(launcher, SIGNAL(aboutToHide()), icon, SLOT(setUnpressed()));
    QObject::connect(launcher, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
}

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      d(new Private)
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");

    setHasConfigurationInterface(true);
    setBackgroundHints(NoBackground);
    d->icon = new Plasma::Icon(KIcon("start-here-kde"), QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
    connect(this, SIGNAL(activate()), this, SLOT(toggleMenu()));
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addItem(d->icon);

    if (KService::serviceByStorageId("kde4-kmenuedit.desktop")) {
        QAction* menueditor = new QAction(i18n("Menu Editor"), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    setAspectRatioMode(Plasma::ConstrainedSquare);

    Q_ASSERT( ! d->switcher );
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->switcher->setVisible(immutability() == Plasma::Mutable);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));
    resize(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop));
    d->icon->resize(contentsRect().size());
}

void LauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    setBackgroundHints(NoBackground);
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            //FIXME set correct minimum size
            //setMinimumSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            //setMinimumSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
        }
    }

    if (constraints & Plasma::ImmutableConstraint && d->switcher) {
        d->switcher->setVisible(immutability() == Plasma::Mutable);
    }
}

void LauncherApplet::switchMenuStyle()
{
    if (containment()) {
        containment()->addApplet("simplelauncher", QVariantList(), geometry());
        destroy();
    }
}

void LauncherApplet::startMenuEditor()
{
    KProcess::execute("kmenuedit");
}

void LauncherApplet::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    widget->setLayout(layout);

    QHBoxLayout *vl = new QHBoxLayout(widget);
    layout->addLayout(vl);

    d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch tabs on hover"), widget);
    layout->addWidget(d->switchOnHoverCheckBox);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), icon());

    if (!d->launcher) {
        d->createLauncher(this);
    }

    d->switchOnHoverCheckBox->setChecked(d->launcher->switchTabsOnHover());
}

void LauncherApplet::configAccepted()
{
    bool switchTabsOnHover = d->switchOnHoverCheckBox->isChecked();

    // TODO: should this be moved into Launcher as well? perhaps even the config itself?
    KConfigGroup cg = config();
    cg.writeEntry("SwitchTabsOnHover", switchTabsOnHover);
    emit configNeedsSaving();

    if (!d->launcher) {
        d->createLauncher(this);
    }

    d->launcher->setSwitchTabsOnHover(switchTabsOnHover);
}


void LauncherApplet::toggleMenu(bool pressed)
{
    if (pressed) {
        toggleMenu();
    }
}

void LauncherApplet::toggleMenu()
{
    if (!d->launcher) {
        d->createLauncher(this);
    }

    d->launcher->reset();

    if (!d->launcher->isVisible()) {
        // It's risky to calculate the popupPosition based on the size before
        // calling setLauncherOrigin, which can change the size
        // Probably just a problem on screens with strange aspect ratio or smaller than
        // kickoff's size.
        QPoint popupPosition = Applet::popupPosition(d->launcher->size());
        d->launcher->move( popupPosition );
        QPoint iconPosition = view()->mapToGlobal(
                view()->mapFromScene( d->icon->scenePos() ) );

        Plasma::View *pv = dynamic_cast<Plasma::View *>(view());
        Plasma::Location loc = Plasma::Floating;
        if (pv) {
            loc = pv->containment()->location();
        }

        d->launcher->setLauncherOrigin( iconPosition, loc );
        emit releaseVisualFocus();
    }

    d->launcher->setVisible(!d->launcher->isVisible());
    d->icon->setPressed();
}

QList<QAction*> LauncherApplet::contextualActions()
{
  return d->actions;
}

#include "applet.moc"
