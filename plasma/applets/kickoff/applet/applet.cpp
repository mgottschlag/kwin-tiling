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

// Plasma
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

// Local
#include "ui/launcher.h"

class LauncherApplet::Private
{
public:
    Plasma::Icon *icon;
    Kickoff::Launcher *launcher;
    bool switchTabsOnHover;
    int visibleItemsCount;
    
    KDialog *dialog;
    QCheckBox *switchOnHoverCheckBox;
    KIntNumInput *visibleCountEdit;

    Private() : launcher(0), dialog(0) {}
    ~Private() { delete dialog; delete launcher; }
};

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      d(new Private)
      
{
    setHasConfigurationInterface(true);
    setRemainSquare(true);

    d->icon = new Plasma::Icon(KIcon("start-here-kde"), QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));

    d->switchTabsOnHover = true;
    d->visibleItemsCount = 10;
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    KConfigGroup cg = config();
    d->switchTabsOnHover = cg.readEntry("SwitchTabsOnHover",d->switchTabsOnHover);
    d->visibleItemsCount = cg.readEntry("VisibleItemsCount",d->visibleItemsCount);
}

void LauncherApplet::constraintsUpdated(Plasma::Constraints constraints)
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
        vl->addWidget(new QLabel(i18n("Number of visible items:"), d->dialog->mainWidget()));
        d->visibleCountEdit = new KIntNumInput(d->dialog->mainWidget());
        d->visibleCountEdit->setMinimum(1);
        d->visibleCountEdit->setSliderEnabled(false);
        vl->addWidget(d->visibleCountEdit);

        d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch tabs on hover"), d->dialog->mainWidget());
        layout->addWidget(d->switchOnHoverCheckBox);
    }
    d->visibleCountEdit->setValue(d->visibleItemsCount);
    d->switchOnHoverCheckBox->setChecked(d->switchTabsOnHover);
    d->dialog->show();
}

Qt::Orientations LauncherApplet::expandingDirections() const
{
    return 0;
}

void LauncherApplet::configAccepted()
{
    d->switchTabsOnHover = d->switchOnHoverCheckBox->isChecked();
    d->visibleItemsCount = d->visibleCountEdit->value();

    KConfigGroup cg = config();
    cg.writeEntry("SwitchTabsOnHover",d->switchTabsOnHover);
    cg.writeEntry("VisibleItemsCount",d->visibleItemsCount);
    emit configNeedsSaving();

    if (d->launcher) {
        d->launcher->setSwitchTabsOnHover(d->switchTabsOnHover);
        if (d->launcher->visibleItemCount() != d->visibleItemsCount) {
            d->launcher->setVisibleItemCount(d->visibleItemsCount);
            d->launcher->adjustSize();
        }
    }
}

void LauncherApplet::toggleMenu(bool pressed)
{
    if (!pressed) {
        return;
    }

    //kDebug() << "Launcher button clicked";
    if (!d->launcher) {
        d->launcher = new Kickoff::Launcher(0);
        d->launcher->setWindowFlags(d->launcher->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
        d->launcher->setAutoHide(true);
        d->launcher->setSwitchTabsOnHover(d->switchTabsOnHover);
        d->launcher->setVisibleItemCount(d->visibleItemsCount);
        d->launcher->adjustSize();
        d->launcher->setApplet(this);
        connect(d->launcher, SIGNAL(aboutToHide()), d->icon, SLOT(setUnpressed()));
    }
    d->launcher->reset();

    if (!d->launcher->isVisible()) {
        d->launcher->move(d->icon->popupPosition(d->launcher->size()));

        if (containment()) {
            containment()->emitLaunchActivated();
        }
    }

    d->launcher->setVisible(!d->launcher->isVisible());
    d->icon->setPressed();
}

#include "applet.moc"
