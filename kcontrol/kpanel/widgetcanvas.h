//
// A special widget which draws a sample of KDE widgets
// It is used to preview color schemes
//
// Copyright (c)  Mark Donohoe 1998
//
// Modified to do a KPanel preview by Daniel M. Duley <mosfet@kde.org>
//

#ifndef __WIDGETCANVAS_H__
#define __WIDGETCANVAS_H__

#include <qpixmap.h>
#include <qdrawutil.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qscrollbar.h>

#include <kapp.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kpixmap.h>

#define MAX_HOTSPOTS   19
#define SCROLLBAR_SIZE 16

class HotSpot
{
public:
    HotSpot() {}
    HotSpot( const QRect &r, int num )
    {	rect = r; number = num; }

    QRect rect;
    int number;
};


#define CANVAS_ITEMS 5

class WidgetCanvas : public QWidget
{
    Q_OBJECT
public:
    enum CanvasWidgets{C_Panel=0, C_Icon, C_TBar, C_TBtn, C_TText};
        
    WidgetCanvas( QWidget *parent=0, const char *name=0 );
    QSize sizeHint() const;
    void drawSampleWidgets();
    QPixmap smplw;

    QColor colors[CANVAS_ITEMS];
    QPixmap pixmaps[CANVAS_ITEMS];
    QPixmap goPix, appPix;
    int boxSize;
signals:
    void widgetSelected( int );
    void colorDropped( int, const QColor&);
protected:
    QColorGroup makeColorGroup(const QColor &fg, const QColor &bg);
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void resizeEvent(QResizeEvent *);
    virtual void dropEvent( QDropEvent *);
    virtual void dragEnterEvent( QDragEnterEvent *);
    void paletteChange( const QPalette & );

    HotSpot hotspots[MAX_HOTSPOTS];
};

#endif
