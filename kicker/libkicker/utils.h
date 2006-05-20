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

#ifndef _plasmautils_h__
#define _plasmautils_h__

#include <Qt>
#include <QMap>
#include <QMenu>

#include <kurl.h>

namespace Plasma
{
    // TODO: these should be moved to a plasma or general definitions header at
    // some point. they don't belong in utils.h really
    enum ScreenEdge { NoEdge = 0, TopEdge, TopRightEdge, RightEdge,
                      BottomRightEdge, BottomEdge, BottomLeftEdge, LeftEdge,
                      TopLeftEdge };
    enum Type { Normal = 0, Stretch };
    enum Action { About = 1, Help = 2, Preferences = 4, ReportBug = 8 };
    enum Position { Left = 0, Right, Up, Down, Top = Up, Bottom = Down, Floating };
    enum Direction { LeftToRight = 0, RightToLeft, TopToBottom, BottomToTop };
    enum Alignment { LeftTop = 0, Center, RightBottom };
    enum Size { SizeTiny = 0, SizeSmall, SizeNormal, SizeLarge, SizeCustom };

    const int XineramaAllScreens = -2;

/*
 * Given a Qt::ArrotwType return the proper Plasma::Position
 */
KDE_EXPORT Position arrowToDirection(Qt::ArrowType p);

/*
 * For a given Plasma::Position, report which way the popups should be pointing
 */
KDE_EXPORT Position popupDirectionForPosition(Position p);

/*
 * Given a Plasma::Sizes value, returns the size in pixels
 */
KDE_EXPORT int sizeValue(Plasma::Size s);

/**
 * Pixel sizes for but sizes and margins
 */
KDE_EXPORT int maxButtonDim();

/**
 * Tint the image to reflect the current color scheme
 * Used, for instance, by KMenu side bar
 */
KDE_EXPORT void colorize(QImage& image);

/**
 * Blend two colours together to get a colour halfway in between
 */
KDE_EXPORT QColor blendColors(const QColor& c1, const QColor& c2);

/**
 * Create or copy .desktop files for use in kicker safely and easily
 */
KDE_EXPORT QString copyDesktopFile(const KUrl& url);
KDE_EXPORT QString newDesktopFile(const KUrl& url);


/**
 * Reduces a popup menu
 *
 * When a popup menu contains only 1 sub-menu, it makes no sense to
 * show this popup-menu but we better show the sub-menu directly.
 *
 * This function checks whether that is the case and returns either the
 * original menu or the sub-menu when appropriate.
 */
KDE_EXPORT QMenu *reduceMenu(QMenu *);


/**
 * Calculate the appropriate position for a popup menu based on the
 * direction, the size of the menu, the widget geometry, and a optional
 * point in the local coordinates of the widget.
 */
KDE_EXPORT QPoint popupPosition(Plasma::Position d,
                                const QWidget* popup,
                                const QWidget* source,
                                const QPoint& offset = QPoint(0, 0));

/**
 * Calculate an acceptable inverse of the given color which will be used
 * as the shadow color.
 */
KDE_EXPORT QColor shadowColor(const QColor& c);


/**
 * Get an appropriate for a menu in Plasma. As the user may set this size
 * globally, it is important to always use this method.
 * @param icon the name of icon requested
 * @return the icon set for the requested icon
 */
KDE_EXPORT QIcon menuIconSet(const QString& icon);

}

#endif // __pglobal_h__
