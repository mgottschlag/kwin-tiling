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

// KDE
#include <KIcon>
#include <KDebug>
#include <KDialog>
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

    KDialog *dialog;
    QCheckBox *switchOnHoverCheckBox;
    KIntNumInput *visibleCountEdit;
    QList<QAction*> actions;
    QAction* switcher;

    Private() : launcher(0), dialog(0), switcher(0) {}
    ~Private() { delete dialog; delete launcher; }
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
    setHasConfigurationInterface(true);
    setRemainSquare(true);
    setDrawStandardBackground(true);
    d->icon = new Plasma::Icon(KIcon("start-here-kde"), QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    if (KService::serviceByStorageId("kde4-kmenuedit.desktop")) {
        QAction* menueditor = new QAction(i18n("Menu Editor"), this);
        d->actions.append(menueditor);
        connect(menueditor, SIGNAL(triggered(bool)), this, SLOT(startMenuEditor()));
    }

    Q_ASSERT( ! d->switcher );
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->switcher->setVisible(immutability() == Plasma::NotImmutable);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));
    resize(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop));
    d->icon->resize(geometry().size());
}

void LauncherApplet::constraintsUpdated(Plasma::Constraints constraints)
{
    setDrawStandardBackground(false);
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
	        setMinimumSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
        } else {
            setMinimumSize(d->icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        d->icon->resize(geometry().size());
    }

    if (constraints & Plasma::ImmutableConstraint) {
        d->switcher->setVisible(immutability() == Plasma::NotImmutable);
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

void LauncherApplet::showConfigurationInterface()
{
    if (! d->dialog) {
        d->dialog = new KDialog();
        d->dialog->setCaption( i18nc("@title:window","Configure Launcher") );
        d->dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect(d->dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
        connect(d->dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()));

        QVBoxLayout *layout = new QVBoxLayout(d->dialog->mainWidget());
        d->dialog->mainWidget()->setLayout(layout);

        QHBoxLayout *vl = new QHBoxLayout(d->dialog->mainWidget());
        layout->addLayout(vl);

        d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch tabs on hover"), d->dialog->mainWidget());
        layout->addWidget(d->switchOnHoverCheckBox);
    }

    if (!d->launcher) {
        d->createLauncher(this);
    }

    d->switchOnHoverCheckBox->setChecked(d->launcher->switchTabsOnHover());
    d->dialog->show();
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
    if (!pressed) {
        return;
    }

    //kDebug() << "Launcher button clicked";
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
        emit launchActivated();
    }

    d->launcher->setVisible(!d->launcher->isVisible());
    d->icon->setPressed();
}

QList<QAction*> LauncherApplet::contextActions()
{
  return d->actions;
}

#include "applet.moc"
