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
#include <QtDebug>

// KDE
#include <KIcon>
#include <KDialog>

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
    
    KDialog *dialog;
    QCheckBox *switchOnHoverCheckBox;

    Private() : launcher(0), dialog(0) {}
    ~Private() { delete dialog; delete launcher; }
};

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      d(new Private)
      
{
    setHasConfigurationInterface(true);
    //setDrawStandardBackground(true);

    Plasma::HBoxLayout *layout = new Plasma::HBoxLayout(this);
    layout->setMargin(0);
    d->icon = new Plasma::Icon(KIcon("start-here"), QString(), this);
    d->icon->setFlag(ItemIsMovable, false);
    connect(d->icon, SIGNAL(pressed(bool)), this, SLOT(toggleMenu(bool)));

    d->switchTabsOnHover = true;
}

LauncherApplet::~LauncherApplet()
{
    delete d;
}

void LauncherApplet::init()
{
    KConfigGroup cg = config();
    d->switchTabsOnHover = cg.readEntry("SwitchTabsOnHover",d->switchTabsOnHover);
}

QSizeF LauncherApplet::contentSizeHint() const
{
    return QSizeF(48,48);
}

Qt::Orientations LauncherApplet::expandingDirections() const
{
    return 0;
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

        d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch Tabs on Hover"), d->dialog->mainWidget());
        d->switchOnHoverCheckBox->setCheckState(d->switchTabsOnHover ? Qt::Checked : Qt::Unchecked);
        layout->addWidget(d->switchOnHoverCheckBox);
    }
    d->dialog->show();
}

void LauncherApplet::configAccepted()
{
    d->switchTabsOnHover = d->switchOnHoverCheckBox->checkState() == Qt::Checked;

    KConfigGroup cg = config();
    cg.writeEntry("SwitchTabsOnHover",d->switchTabsOnHover);
    cg.config()->sync();

    if (d->launcher) {
        d->launcher->setSwitchTabsOnHover(d->switchTabsOnHover);
    }
}

void LauncherApplet::toggleMenu(bool pressed)
{
    if (!pressed) {
        return;
    }

    //qDebug() << "Launcher button clicked";
    if (!d->launcher) {
        d->launcher = new Kickoff::Launcher(0);
        d->launcher->setWindowFlags(d->launcher->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
        d->launcher->setAutoHide(true);
        d->launcher->setSwitchTabsOnHover(d->switchTabsOnHover);
        d->launcher->adjustSize();
        connect(d->launcher, SIGNAL(aboutToHide()), d->icon, SLOT(setUnpressed()));
    }

    // try to position the launcher alongside the top or bottom edge of the
    // applet with and aligned to the left or right of the applet
    if (!d->launcher->isVisible()) {
        QGraphicsView *viewWidget = view();
        QDesktopWidget *desktop = QApplication::desktop();
        if (viewWidget) {
            QPoint viewPos = viewWidget->mapFromScene(scenePos());
            QPoint globalPos = viewWidget->mapToGlobal(viewPos);
            QRect desktopRect = desktop->availableGeometry(viewWidget);
            QRect size = mapToView(viewWidget, contentRect());
            // Prefer to open below the icon so as to act like a regular menu
            if (globalPos.y() + size.height() + d->launcher->height()
                < desktopRect.bottom()) {
                globalPos.ry() += size.height();
            } else {
                globalPos.ry() -= d->launcher->height();
            }
            if (globalPos.x() + d->launcher->width() > desktopRect.right()) {
                globalPos.rx() -= d->launcher->width() - size.width();
            }
            d->launcher->move(globalPos);
        }
        if (containment()) {
            containment()->emitLaunchActivated();
        }
    }

    d->launcher->setVisible(!d->launcher->isVisible());
    d->icon->setPressed();
}

#include "applet.moc"
