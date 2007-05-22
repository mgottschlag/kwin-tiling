#include "desktop.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
#include <QGLWidget>
#include <KWindowSystem>
#include <KMenu>

#include "clock.h"
#include "dataengine.h"
#include "plasmaapp.h"
#include "svg.h"
#include "widgets/lineedit.h"
#include "applet.h"
#include "desktop.moc"

Desktop::Desktop(QWidget *parent)
    : QGraphicsView(parent)
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

    // Tmp
//     for (int i = 0; i < 4000; i++)
//     {
//         int x = qrand() % 475;
//         int y = qrand() % 475;
// 
//         Plasma::PushButton *testButton = new Plasma::PushButton;
//         testButton->setText(QString::number(i));
//         testButton->moveBy(x, y);
// 
//         m_graphicsScene->addItem(testButton);
//     }

    Clock *clock = new Clock;
    m_graphicsScene->addItem(clock);

    Plasma::DataEngine* time = PlasmaApp::self()->loadDataEngine("time");
    Plasma::LineEdit* l = new Plasma::LineEdit;
    m_graphicsScene->addItem(l);
    l->moveBy(400, 400);
    time->connectSource("time", l);

    m_background = new Plasma::Svg("background/dialog", this);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayContextMenu(QPoint)));
}

Desktop::~Desktop()
{
}

void Desktop::drawBackground(QPainter * painter, const QRectF &)
{
    m_background->paint(painter, rect());
}

void Desktop::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    m_graphicsScene->setSceneRect(rect());
    m_background->resize(width(), height());
}

void Desktop::displayContextMenu(const QPoint& point)
{
    Plasma::Applet* applet = qgraphicsitem_cast<Plasma::Applet*>( itemAt( point ) );
    KMenu* desktopMenu = new KMenu(this);
    if(!applet) {
        desktopMenu->setTitle("Desktop");
        desktopMenu->addAction("The");
        desktopMenu->addAction("desktop");
        desktopMenu->addAction("menu");
    } else {
        //desktopMenu->setTitle( applet->name() ); //This isn't implemented in Applet yet...
        desktopMenu->addAction("Widget");
        desktopMenu->addAction("settings");
        desktopMenu->addAction("like");
        desktopMenu->addAction("opacity");
        desktopMenu->addSeparator();
        //applet->addAppletMenuEntries(desktopMenu); //This isn't implemented in Applet yet...
    }
    desktopMenu->exec(point);
}

