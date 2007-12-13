/*
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
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

#ifndef PLASMA_DESKTOP_H
#define PLASMA_DESKTOP_H

#include <QGraphicsItem>
#include <QList>
#include <QObject>
#include <QStyleOptionGraphicsItem>
#include <QTimer>

#include <KDialog>
#include <KIcon>

#include <plasma/containment.h>
#include <plasma/phase.h>
#include <plasma/widgets/widget.h>

#include "iconloader.h"

#include "renderthread.h"

class BackgroundDialog;
class QAction;
class QTimeLine;

namespace Plasma
{
    class AppletBrowser;
    class Svg;
}

namespace Ui
{
    class config;
}

/*class Tool : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Tool(QGraphicsItem *parent = 0);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

};*/

class DefaultDesktop : public Plasma::Containment
{
    Q_OBJECT

public:
    DefaultDesktop(QObject *parent, const QVariantList &args);
    ~DefaultDesktop();
    void init();
    void constraintsUpdated(Plasma::Constraints constraints);

    QList<QAction*> contextActions();

    /**
     * Paints a default background image. Nothing fancy, but that's what plugins
     * are for. Reimplemented from Plasma::Containment;
     */
    void paintInterface(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        const QRect& contentsRect);

protected Q_SLOTS:
    void runCommand();
    void configure();
    void applyConfig();
    void nextSlide();
    
    void toggleDesktopImmutability();
    void lockScreen();
    void logout();

    void updateBackground();
    void updateBackground(int, const QImage &img);
private:
    void reloadConfig();
    QSize resolution() const;

    QAction *m_lockDesktopAction;
    QAction *m_appletBrowserAction;
    QAction *m_runCommandAction;
    QAction *m_setupDesktopAction;
    QAction *m_lockScreenAction;
    QAction *m_logoutAction;
    QAction *m_separator;

    BackgroundDialog *m_configDialog;
    
    int m_backgroundMode;
    // slideshow settings

    // the index of which m_slidePath is currently visible
    int m_currentSlide;
    QTimer m_slideshowTimer;
    QStringList m_slideFiles;

    QPixmap m_bitmapBackground;
    QString m_wallpaperPath;

    //Desktop icons
    IconLoader m_icons;

    int m_wallpaperPosition;
    QColor m_wallpaperColor;
    
    RenderThread m_renderer;
    int m_current_renderer_token;
};

#endif // PLASMA_PANEL_H
