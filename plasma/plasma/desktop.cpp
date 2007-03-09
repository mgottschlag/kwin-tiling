#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

#include <KWin>

#include "clock.h"
#include "desktop.h"
#include "desktop.moc"

Desktop::Desktop(QWidget *parent)
    : QGraphicsView(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Desktop);

    int primaryScreen = QApplication::desktop()->primaryScreen();
    QRect desktopSize = QApplication::desktop()->screenGeometry(primaryScreen);
    setGeometry(desktopSize);

    m_graphicsScene = new QGraphicsScene(desktopSize);
    m_graphicsScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(m_graphicsScene);
    setRenderHint(QPainter::Antialiasing, false);
    setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Give it some silly default background
    QPixmap tile(100, 100);
    tile.fill(Qt::white);
    QPainter pt(&tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();
    setBackground(tile);

    // Make us legit via KWin
    KWin::setType(winId(), NET::Desktop);
    KWin::setState(winId(), NET::SkipPager);
    KWin::setOnAllDesktops(winId(), true);

    // Tmp
    for (int i = 0; i < 10; i++)
    {
        Plasma::Clock *testClock = new Plasma::Clock;
        m_graphicsScene->addItem(testClock);
    }
}

Desktop::~Desktop()
{
}

void Desktop::setBackground(const QString &path)
{
    QPixmap image(path);
    setBackground(image);
}

void Desktop::setBackground(const QPixmap &image)
{
    resetCachedContent();
    setBackgroundBrush(image);
}

