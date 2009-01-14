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
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QGraphicsLinearLayout>

// KDE
#include <KIcon>
#include <KDebug>
#include <KConfigDialog>
#include <KNumInput>
#include <KProcess>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Containment>
#include <Plasma/View>
#include <Plasma/ToolTipManager>

// Local
#include "ui/launcher.h"
#include "core/recentapplications.h"

class LauncherApplet::Private
{
public:
    Private(LauncherApplet *lApplet) : launcher(0), switcher(0), q(lApplet) { }
    ~Private() {
        delete launcher;
    }
    void createLauncher();
    void initToolTip();

    Kickoff::Launcher *launcher;

    QCheckBox *switchOnHoverCheckBox;
    QSpinBox *recentApplicationsSpinBox;
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
    launcher->setAttribute(Qt::WA_NoSystemBackground);
    launcher->setAutoHide(true);
    QObject::connect(launcher, SIGNAL(aboutToHide()), q, SLOT(hidePopup()));
    //launcher->resize(launcher->sizeHint());
    //QObject::connect(launcher, SIGNAL(aboutToHide()), icon, SLOT(setUnpressed()));
    //QObject::connect(launcher, SIGNAL(configNeedsSaving()), q, SIGNAL(configNeedsSaving()));
}

void LauncherApplet::Private::initToolTip()
{
    Plasma::ToolTipContent data(i18n("Kickoff Application Launcher"),
                                i18n("Favorites, applications, computer places, "
                                     "recently used items and desktop sessions"),
                                q->popupIcon().pixmap(IconSize(KIconLoader::Desktop)));
    Plasma::ToolTipManager::self()->setContent(q, data);
}

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
        : Plasma::PopupApplet(parent, args),
        d(new Private(this))
{
    KGlobal::locale()->insertCatalog("plasma_applet_launcher");
    setHasConfigurationInterface(true);
    setPopupIcon("start-here-kde");
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

    Q_ASSERT(! d->switcher);
    d->switcher = new QAction(i18n("Switch to Classic Menu Style"), this);
    d->actions.append(d->switcher);
    connect(d->switcher, SIGNAL(triggered(bool)), this, SLOT(switchMenuStyle()));

    constraintsEvent(Plasma::ImmutableConstraint);
    Plasma::ToolTipManager::self()->registerWidget(this);
}

void LauncherApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints & Plasma::ImmutableConstraint) && d->switcher) {
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
    QWidget *widget = new QWidget(parent);
    QVBoxLayout *widgetLayout = new QVBoxLayout(widget);
    widget->setLayout(widgetLayout);

    d->switchOnHoverCheckBox = new QCheckBox(i18n("Switch tabs on hover"), widget);
    widgetLayout->addWidget(d->switchOnHoverCheckBox);

    QHBoxLayout *recentLayout = new QHBoxLayout(widget);
    QLabel *recentLabel = new QLabel(i18n("Recent Applications:"), widget);
    recentLayout->addWidget(recentLabel);
    d->recentApplicationsSpinBox = new QSpinBox(widget);
    d->recentApplicationsSpinBox->setMaximum(10);
    d->recentApplicationsSpinBox->setMinimum(0);
    recentLayout->addWidget(d->recentApplicationsSpinBox);
    widgetLayout->addLayout(recentLayout);

    widgetLayout->addStretch();

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, i18n("General"), icon());

    d->createLauncher();
    d->switchOnHoverCheckBox->setChecked(d->launcher->switchTabsOnHover());
    d->recentApplicationsSpinBox->setValue(Kickoff::RecentApplications::self()->maximum());
}

void LauncherApplet::popupEvent(bool show)
{
    if (show) {
        Plasma::ToolTipManager::self()->clearContent(this);
        d->launcher->setLauncherOrigin(popupPlacement(), location());
        d->createLauncher();
    }
}

void LauncherApplet::toolTipAboutToShow()
{
    if (d->launcher->isVisible()) {
        Plasma::ToolTipManager::self()->clearContent(this);
    } else {
        d->initToolTip();
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
    
    Kickoff::RecentApplications::self()->setMaximum(d->recentApplicationsSpinBox->value());
}

QList<QAction*> LauncherApplet::contextualActions()
{
    return d->actions;
}

QWidget *LauncherApplet::widget()
{
    d->createLauncher();
    return d->launcher;
}

#include "applet.moc"
