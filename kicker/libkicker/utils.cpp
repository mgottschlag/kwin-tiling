/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QApplication>
#include <QFile>
#include <QRegExp>
#include <QDesktopWidget>
#include <QMenuItem>

#include <kiconeffect.h>
#include <kiconloader.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kservice.h>

#include "utils.h"
#include "kickerSettings.h"

namespace Plasma
{

Position arrowToDirection(Qt::ArrowType p)
{
    switch (p)
    {
        case Qt::DownArrow:
            return Plasma::Down;

        case Qt::LeftArrow:
            return Plasma::Left;

        case Qt::RightArrow:
            return Plasma::Right;

        case Qt::UpArrow:
        default:
            return Plasma::Up;
    }
}

Position popupDirectionForPosition(Position p)
{
    switch (p)
    {
        case Left:
            return Right;

        case Right:
            return Left;

        case Up:
            return Down;

        case Down:
            return Up;

        case Floating:
            return Right; // FIXME?
    }
	return Right; //FIXME ?
}

int sizeValue(Plasma::Size s)
{
    switch (s)
    {
        case Plasma::SizeTiny:
            return 24;

        case Plasma::SizeSmall:
            return 30;

        case Plasma::SizeNormal:
            return 46;

        case Plasma::SizeLarge:
        default:
            return 58;
    }
}

int maxButtonDim()
{
    return (2 * KickerSettings::iconMargin()) + K3Icon::SizeLarge;
}

QString newDesktopFile(const KUrl& url)
{
   QString base = url.fileName();
   if (base.endsWith(".desktop"))
      base.truncate(base.length()-8);
   QRegExp r("(.*)(?=-\\d+)");
   if (r.search(base) > -1)
      base = r.cap(1);

   QString file = base + ".desktop";

   for(int n = 1; ++n; )
   {
      QString path = locate("appdata", file);
      if (path.isEmpty())
         break;

      file = QString("%2-%1.desktop").arg(n).arg(base);
   }
   file = locateLocal("appdata", file);
   return file;
}

QString copyDesktopFile(const KUrl& url)
{
   QString file = newDesktopFile(url);
   KUrl dest;
   dest.setPath(file);
   KIO::NetAccess::upload(url.path(), dest, 0);
   return file;
}

QMenu* reduceMenu(QMenu *menu)
{
    if (menu->count() != 1)
    {
       return menu;
    }

    QMenuItem *item = menu->findItem(menu->idAt(0));

    if (item->menu())
    {
       return reduceMenu(item->menu());
    }

    return menu;
}

QPoint popupPosition(Plasma::Position d,
                     const QWidget* popup,
                     const QWidget* source,
                     const QPoint& offset)
{
    QRect r;
    if (source->isTopLevel())
    {
        r = source->geometry();
    }
    else
    {
        r = QRect(source->mapToGlobal(QPoint(0, 0)),
                  source->mapToGlobal(QPoint(source->width(), source->height())));

        switch (d)
        {
            case Plasma::Left:
            case Plasma::Right:
                r.setLeft( source->topLevelWidget()->x() );
                r.setWidth( source->topLevelWidget()->width() );
                break;
            case Plasma::Up:
            case Plasma::Down:
                r.setTop( source->topLevelWidget()->y() );
                r.setHeight( source->topLevelWidget()->height() );
                break;
            default:
                //badbadbad - should never happen
                break;
        }
    }

    switch (d)
    {
        case Plasma::Left:
        case Plasma::Right:
        {
            QDesktopWidget* desktop = QApplication::desktop();
            QRect screen = desktop->screenGeometry(desktop->screenNumber(const_cast<QWidget*>(source)));
            int x = (d == Plasma::Left) ? r.left() - popup->width() :
                                          r.right() + 1;
            int y = r.top() + offset.y();

            // try to keep this on screen
            if (y + popup->height() > screen.bottom())
            {
                y = r.bottom() - popup->height() + offset.y();

                if (y < screen.top())
                {
                    y = screen.bottom() - popup->height();

                    if (y < screen.top())
                    {
                        y = screen.top();
                    }
                }
            }

            return QPoint(x, y);
        }
        case Plasma::Up:
        case Plasma::Down:
        default:
        {
            int x = 0;
            int y = (d == Plasma::Up) ? r.top() - popup->height() :
                                        r.bottom() + 1;

            if (QApplication::isRightToLeft())
            {
                x = r.right() - popup->width() + 1;

                if (offset.x() > 0)
                {
                    x -= r.width() - offset.x();
                }

                // try to keep this on the screen
                if (x - popup->width() < 0)
                {
                    x = r.left();
                }

                return QPoint(x, y);
            }
            else
            {
                QDesktopWidget* desktop = QApplication::desktop();
                QRect screen = desktop->screenGeometry(desktop->screenNumber(const_cast<QWidget*>(source)));
                x = r.left() + offset.x();

                // try to keep this on the screen
                if (x + popup->width() > screen.right())
                {
                    x = r.right() - popup->width() + 1 + offset.x();

                    if (x < screen.left())
                    {
                        x = screen.left();
                    }
                }
            }

            return QPoint(x, y);
        }
    }
}

void colorize(QImage& image)
{
    KConfig *config = KGlobal::config();
    config->setGroup("WM");
    QColor color = QApplication::palette().active().highlight();
    QColor activeTitle = config->readEntry("activeBackground", color);
    QColor inactiveTitle = config->readEntry("inactiveBackground", color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.getHsv(&h1, &s1, &v1);
    inactiveTitle.getHsv(&h2, &s2, &v2);
    QApplication::palette().active().background().getHsv(&h3, &s3, &v3);

    if ( (qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < qAbs(h2-h3)+qAbs(s2-s3)+qAbs(v2-v3)) &&
            ((qAbs(h1-h3)+qAbs(s1-s3)+qAbs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int r, g, b;
    color.getRgb(&r, &g, &b);
    int gray = qGray(r, g, b);
    if (gray > 180) {
        r = (r - (gray - 180) < 0 ? 0 : r - (gray - 180));
        g = (g - (gray - 180) < 0 ? 0 : g - (gray - 180));
        b = (b - (gray - 180) < 0 ? 0 : b - (gray - 180));
    } else if (gray < 76) {
        r = (r + (76 - gray) > 255 ? 255 : r + (76 - gray));
        g = (g + (76 - gray) > 255 ? 255 : g + (76 - gray));
        b = (b + (76 - gray) > 255 ? 255 : b + (76 - gray));
    }
    color.setRgb(r, g, b);
    KIconEffect::colorize(image, color, 1.0);
}

QColor blendColors(const QColor& c1, const QColor& c2)
{
    int r1, g1, b1;
    int r2, g2, b2;

    c1.getRgb(&r1, &g1, &b1);
    c2.getRgb(&r2, &g2, &b2);

    r1 += (int) (.5 * (r2 - r1));
    g1 += (int) (.5 * (g2 - g1));
    b1 += (int) (.5 * (b2 - b1));

    return QColor(r1, g1, b1);
}

QColor shadowColor(const QColor& c)
{
    int r = c.red();
    int g = c.green();
    int b = c.blue();

    if ( r < 128 )
        r = 255;
    else
        r = 0;

    if ( g < 128 )
        g = 255;
    else
        g = 0;

    if ( b < 128 )
        b = 255;
    else
        b = 0;

    return QColor( r, g, b );
}

QIcon menuIconSet(const QString& icon)
{
    QIcon iconset;
    int iconSize = KickerSettings::menuEntryHeight();
    if (iconSize > 0)
    {
        iconset = KGlobal::iconLoader()->loadIconSet(icon,
                                                     K3Icon::NoGroup,
                                                     iconSize);
    }
    else if (iconSize == 0)
    {
        QPixmap normal = KGlobal::iconLoader()->loadIcon(icon,
                                                         K3Icon::Small,
                                                         0,
                                                         K3Icon::DefaultState,
                                                         0,
                                                         true);

        QPixmap active = KGlobal::iconLoader()->loadIcon(icon,
                                                         K3Icon::Small,
                                                         0,
                                                         K3Icon::ActiveState,
                                                         0,
                                                         true);
        // make sure they are not larger than 20x20
        if (normal.width() > 20 || normal.height() > 20)
        {
            normal.convertFromImage(normal.convertToImage().smoothScale(20,20));
        }

        if (active.width() > 20 || active.height() > 20)
        {
            active.convertFromImage(active.convertToImage().smoothScale(20,20));
        }

        iconset.setPixmap(normal, QIcon::Small, QIcon::Normal);
        iconset.setPixmap(active, QIcon::Small, QIcon::Active);
    }

    return iconset;
}

} // namespace

