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
#include <plasma/tooltipmanager.h>

// Local
#include "ui/launcher.h"

class LauncherApplet::Private
{
public:
    Private(LauncherApplet *lApplet) : launcher(0), switcher(0), q(lApplet) { }
    ~Private() { delete launcher; }
    void createLauncher();
    void initToolTip();

    Kickoff::Launcher *launcher;

    QCheckBox *switchOnHoverCheckBox;
    KIntNumInput *visibleCountEdit;
    QList<QAction*> actions;
    QAction* switcher;
    LauncherApplet *q;
};

void LauncherApplet::Private::createLauncher()
{
    if (launcher) {
        return;
    }

    launcher = new Kickoff::Launcher(q);
    launcher->setAutoHide(true);
    QObject::connect(launcher, SIGNAL(aboutToHide()), q, SLOT(hidePopup()));
    //launcher->resize(launcher->sizeHint());
    //QObject::connect(launcher, SIGNAL(aboutToHide()), icon, SLOT(setUnpressed()));
    //QObject::connect(launcher, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
}

void LauncherApplet::Private::initToolTip()
{
    Plasma::ToolTipManager::self()->registerWidget(q);
    Plasma::ToolTipManager::ToolTipContent data;
    data.mainText = i18n("Kickoff Application Launcher");
    data.subText = i18n("Favorites, applications, computer places, recently used items and desktop sessions");
    data.image = q->icon().pixmap(IconSize(KIconLoader::Desktop));
    Plasma::ToolTipManager::self()->setToolTipContent(q, data);
}

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent,args),
      d(new Private(this))
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");
    setHasConfigurationInterface(true);
    setIcon("start-here-kde");
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

    setAspectRatioMode(Plasma::ConstrainedSquare);

    Q_ASSERT( ! d->switcher );
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->switcher->setVisible(immutability() == Plasma::Mutable);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    d->initToolTip();
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
    parent->addPage(widget, parent->windowTitle()/*, icon()*/);

    d->createLauncher();
    d->switchOnHoverCheckBox->setChecked(d->launcher->switchTabsOnHover());
}

void LauncherApplet::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->setToolTipActivated(this, false);
        d->createLauncher();
        QPoint iconPosition = view()->mapToGlobal(view()->mapFromScene(scenePos()));
        d->launcher->setLauncherOrigin(iconPosition, location());
    } else {
        Plasma::ToolTipManager::self()->setToolTipActivated(this, true);
    }
}

void LauncherApplet::configAccepted()
{
    bool switchTabsOnHover = d->switchOnHoverCheckBox->isChecked();

    // TODO: should this be moved into Launcher as well? perhaps even the config itself?
    KConfigGroup cg = globalConfig();
    cg.writeEntry("SwitchTabsOnHover", switchTabsOnHover);
    emit configNeedsSaving();

    d->createLauncher();
    d->launcher->setSwitchTabsOnHover(switchTabsOnHover);
}

QList<QAction*> LauncherApplet::contextualActions()
{
  return d->actions;
}

QWidget *LauncherApplet::widget()
{
    d->createLauncher();
    d->launcher->setAttribute(Qt::WA_NoSystemBackground);
    return d->launcher;
}

#include "applet.moc"
