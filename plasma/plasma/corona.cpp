/*
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QGLWidget>
#include <QProcess>
#include <QContextMenuEvent>
#include <KWindowSystem>
#include <KMenu>
#include <KLocale>

#include <KLibrary>
#include <KLibLoader>
#include <QFile>

#include "applet.h"
#include "dataengine.h"
#include "svg.h"
#include "widgets/vboxlayout.h"

#include "corona.h"
#include "plasmaapp.h"

using namespace Plasma;
extern "C" {
    typedef QGraphicsItem* (*loadKaramba)(const KUrl &theme, QGraphicsView *view);
}

Corona::Corona(QWidget *parent)
    : QGraphicsView(parent),
      m_formFactor(Plasma::Planar),
      m_location(Plasma::Floating),
      m_layout(0)
{
    setFrameShape(QFrame::NoFrame);
/*    setBackgroundMode(Qt::NoBackground);*/
    setAutoFillBackground(true);

/*    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pal);*/
    //setViewport(new QGLWidget  ( QGLFormat(QGL::StencilBuffer | QGL::AlphaChannel)   ));

    m_graphicsScene = new QGraphicsScene(rect());
    m_graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(m_graphicsScene);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /*
    KPluginInfo::List applets = Plasma::Applet::knownApplets();
    kDebug() << "======= Applets =========" << endl;
    foreach (KPluginInfo* info, applets) {
        kDebug() << info->pluginName() << ": " << info->name() << endl;
    }
    kDebug() << "=========================" << endl;
    */

    // Loading SuperKaramba themes example
    /*
    QString karambaLib = QFile::encodeName(KLibLoader::self()->findLibrary(QLatin1String("libsuperkaramba")));
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library(karambaLib);

    if (lib) {
        loadKaramba startKaramba = 0;
        startKaramba = (loadKaramba)lib->resolveFunction("startKaramba");
        if (startKaramba) {
            startKaramba(KUrl("~/themes/aero_aio.skz"), this);
        }

        lib->unload();
    }
    */

    m_background = new Plasma::Svg("background/dialog", this);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayContextMenu(QPoint)));
    m_engineExplorerAction = new QAction(i18n("Engine Explorer"), this);
    connect(m_engineExplorerAction, SIGNAL(triggered(bool)), this, SLOT(launchExplorer(bool)));
    setContextMenuPolicy(Qt::CustomContextMenu);
}

Corona::~Corona()
{
    while(!m_applets.isEmpty())
        delete m_applets.takeFirst();
}

Plasma::Location Corona::location() const
{
    return m_location;
}

void Corona::setLocation(Plasma::Location location)
{
    m_location = location;
}

Plasma::FormFactor Corona::formFactor() const
{
    return m_formFactor;
}

void Corona::setFormFactor(Plasma::FormFactor formFactor)
{
    if (m_formFactor == formFactor) {
        return;
    }

    m_formFactor = formFactor;
    delete m_layout;
    m_layout = 0;

    switch (m_formFactor) {
        case Plasma::Planar:
            break;
        case Plasma::Horizontal:
            //m_layout = new Plasma::HBoxLayout;
            break;
        case Plasma::Vertical:
            m_layout = new Plasma::VBoxLayout;
            break;
        case Plasma::MediaCenter:
            break;
        default:
            kDebug() << "This can't be happening!" << endl;
            break;
    }

    if (!m_layout) {
        return;
    }

    foreach (Applet* applet, m_applets) {
        //FIXME: the applet needs a way to query for its constraints
        //       currently there is no framework for this!
        //       so we tell them the constraints are updated, and they should
        //       in turn query for what they are and do any layouting changes
        //       they need to.....
        applet->constraintsUpdated();
    }
}


void Corona::addPlasmoid(const QString& name)
{
    Plasma::Applet* applet = Plasma::Applet::loadApplet(name);
    if (applet) {
        m_graphicsScene->addItem(applet);
        m_applets << applet;
    } else {
        kDebug() << "Plasmoid " << name << " could not be loaded." << endl;
    }
}

void Corona::drawBackground(QPainter * painter, const QRectF &)
{
    m_background->paint(painter, rect());
}

void Corona::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_graphicsScene->setSceneRect(rect());
    m_background->resize(width(), height());
}

void Corona::displayContextMenu(const QPoint& point)
{
    /*
     * example for displaying the SuperKaramba context menu
    QGraphicsItem *item = itemAt(point)->parentItem();
    QObject *object = dynamic_cast<QObject*>(item);
    if(object && object->objectName().startsWith("karamba")) {
        QContextMenuEvent event(QContextMenuEvent::Mouse, point);
        contextMenuEvent(&event);
        return;
    }
    */
    Plasma::Applet* applet = qgraphicsitem_cast<Plasma::Applet*>( itemAt( point ) );
    KMenu desktopMenu(this);
    if(!applet) {
        desktopMenu.setTitle("Corona");
        desktopMenu.addAction("The");
        desktopMenu.addAction("desktop");
        desktopMenu.addAction("menu");
        desktopMenu.addAction(m_engineExplorerAction);
    } else {
        //desktopMenu.setTitle( applet->name() ); //This isn't implemented in Applet yet...
        desktopMenu.addAction("Widget");
        desktopMenu.addAction("settings");
        desktopMenu.addAction("like");
        desktopMenu.addAction("opacity");
        desktopMenu.addSeparator();
        QAction* configureApplet = new QAction(i18n("Configure Applet..."), this);
        connect(configureApplet, SIGNAL(triggered(bool)),
                applet, SLOT(configureDialog())); //This isn't implemented in Applet yet...
        desktopMenu.addAction(configureApplet);
    }
    desktopMenu.exec(point);
}

void Corona::launchExplorer(bool /*param*/)
{
    QProcess::execute("plasmaengineexplorer");
}

#include "corona.moc"

