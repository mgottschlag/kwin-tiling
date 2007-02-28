#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

#include <KWin>

#include "desktop.h"
#include "desktop.moc"

Desktop::Desktop(QWidget *parent)
    : QWidget(parent)
{
    int primaryScreen = QApplication::desktop()->primaryScreen();
    QRect desktopSize = QApplication::desktop()->screenGeometry(primaryScreen);
    setGeometry(desktopSize);

    m_graphicsView = new QGraphicsView;
    m_graphicsScene = new QGraphicsScene(desktopSize);
    m_graphicsView->setScene(m_graphicsScene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing, false);
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

    // Give it some silly default background
    QPixmap tile(100, 100);
    tile.fill(Qt::white);
    QPainter pt(&tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();
    m_graphicsView->setBackgroundBrush(tile);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_graphicsView);

    // Make us legit via KWin
    KWin::setType( winId(), NET::Desktop );
    KWin::setState( winId(), NET::SkipPager );
    KWin::setOnAllDesktops( winId(), true );
}



