//
// A special widget which draws a sample of KDE widgets
// It is used to preview color schemes
//
// Copyright (c)  Mark Donohoe 1998
//
// Modified to do a simple KPanel by Daniel M. Duley <mosfet@kde.org>
//

#include <kcolordrag.h>
#include "widgetcanvas.h"
#include "widgetcanvas.moc"

WidgetCanvas::WidgetCanvas( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    setAcceptDrops( true);
    smplw.resize(100, boxSize+25);
}

void WidgetCanvas::paintEvent(QPaintEvent *)
{
    bitBlt( this, 0, 0, &smplw );
}

void WidgetCanvas::resizeEvent(QResizeEvent *)
{
    smplw.resize(size());
    drawSampleWidgets();
}

void WidgetCanvas::mousePressEvent( QMouseEvent *me )
{
    for ( int i = 0; i < MAX_HOTSPOTS; i++ )
        if ( hotspots[i].rect.contains( me->pos() ) )
        {
            emit widgetSelected( hotspots[i].number );
            return;
        }
}

void WidgetCanvas::dropEvent( QDropEvent *e)
{
    QColor c;
    if( KColorDrag::decode( e, c)) {
        for ( int i = 0; i < MAX_HOTSPOTS; i++ )
            if ( hotspots[i].rect.contains( e->pos() ) ) {
                emit colorDropped( hotspots[i].number, c);
                return;
            }
    }
}

void WidgetCanvas::dragEnterEvent( QDragEnterEvent *e)
{
    e->accept( KColorDrag::canDecode( e));
}

void WidgetCanvas::paletteChange(const QPalette &)
{
    drawSampleWidgets();
}

void WidgetCanvas::drawSampleWidgets()
{
    int tbarH = 23;
    int spot = 0;
    QBrush brush;
    QColorGroup cg;

    QPainter paint( &smplw );

    // Draw main kpanel
    brush = QBrush(colors[C_Panel]);
    cg = makeColorGroup(Qt::black, colors[C_Panel]);
    if(!pixmaps[C_Panel].isNull())
        brush.setPixmap(pixmaps[C_Panel]);
    qDrawShadePanel(&paint, 0, tbarH, width(), boxSize+2, cg, FALSE, 1,
                    &brush);
 
    // Draw panel app icon
    if(!pixmaps[C_Icon].isNull())
        paint.drawTiledPixmap(15, tbarH+1, boxSize, boxSize, pixmaps[C_Icon]);
    if(!goPix.isNull()){
        paint.setClipRect(15, tbarH+1, boxSize, boxSize);
        // Dont count on icon being 32x32 ;-)
        int xOffset = (boxSize - goPix.width())/2;
        int yOffset = (boxSize - goPix.height())/2;
        paint.drawPixmap(15 + (xOffset > 0 ? xOffset : 0),
                         tbarH+1 + (yOffset > 0 ? yOffset : 0), goPix);
        paint.setClipping(false);
    }
    hotspots[ spot++ ] =
        HotSpot(QRect(15, tbarH+1, boxSize, boxSize), C_Icon);
    hotspots[ spot++ ] =
        HotSpot(QRect( 0, tbarH, width(), boxSize+2 ), C_Panel);

    // Draw taskbar
    brush = QBrush(colors[C_TBar]);
    cg = makeColorGroup(Qt::black, colors[C_TBar]);
    if(!pixmaps[C_TBar].isNull())
        brush.setPixmap(pixmaps[C_TBar]);
    qDrawWinPanel(&paint, 0, 0, width(), tbarH, cg, FALSE, &brush);

    // Draw taskbar button
    int textLen = paint.fontMetrics().width(i18n("Cool KDE App")) + 40;
    brush = QBrush(colors[C_TBtn]);
    cg = makeColorGroup(Qt::black, colors[C_TBtn]);
    if(!pixmaps[C_TBtn].isNull())
        brush.setPixmap(pixmaps[C_TBtn]);
    qDrawWinPanel(&paint, 0, 0, textLen, tbarH, cg, false, &brush);
    if(!appPix.isNull())
        paint.drawPixmap(8, (tbarH - appPix.height())/2, appPix);
    
    paint.setPen(colors[C_TText]);
    paint.drawText(28, 0, textLen-28, tbarH, AlignLeft | AlignVCenter,
                   i18n("Cool KDE App"));
    hotspots[ spot++ ] =
        HotSpot(QRect(28, 0, textLen-28, tbarH), C_TText);
    hotspots[ spot++ ] =
        HotSpot(QRect(0, 0, textLen, tbarH), C_TBtn);
    hotspots[ spot++ ] =
        HotSpot(QRect(0, 0, width(), tbarH), C_TBar);
}

QColorGroup WidgetCanvas::makeColorGroup(const QColor &fg, const QColor &bg)
{
    return(QColorGroup( fg, bg, bg.light(150), bg.dark(),
                        bg.dark(120), fg,
                        kapp->palette()->normal().base()));
}

QSize WidgetCanvas::sizeHint() const
{
    return(QSize(100, boxSize+2+23));
}
