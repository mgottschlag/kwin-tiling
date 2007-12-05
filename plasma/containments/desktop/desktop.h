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

#include <KDialog>
#include <KIcon>

#include <plasma/containment.h>
#include <plasma/phase.h>
#include <plasma/widgets/widget.h>

class QAction;
class QTimer;
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
    void updateSlideList();
    void nextSlide();

    /** add new path button was clicked */
    void addSlidePath();

    /** add a new slide path */
    void addPathOk();

    /** remove the currently selected path */
    void removeSlidePath();

    /** the slidepath selection has changed,
     * used to enable/disable the remove button */
    void slidePathCurrentRowChanged(int row);

    /** move the selected path up in the list */
    void movePathDown();

    /** move the selected path down in the list */
    void movePathUp();

    /**
     * invoke kns dialog to get new wallpapers
     */
    void getNewStuff();
    
    void toggleDesktopImmutability();
    void lockScreen();
    void logout();

private:
    /** populate m_bitmapBackground with the pixmap to show */
    void getBitmapBackground();

    QAction *m_lockDesktopAction;
    QAction *m_appletBrowserAction;
    QAction *m_runCommandAction;
    QAction *m_setupDesktopAction;
    QAction *m_lockScreenAction;
    QAction *m_logoutAction;

    KDialog *m_configDialog;
    Ui::config *m_ui;

    // IMPORTANT: this needs to be in the same order as the items
    // in the m_ui->pictureComboBox
    enum BackgroundMode {
        kStaticBackground,
        kSlideshowBackground
    };

    int m_backgroundMode;
    // slideshow settings

    // the index of which m_slidePath is currently visible
    int m_currentSlide;
    QTimer *m_slideShowTimer;
    QStringList m_slidePaths;
    QStringList m_slideFiles;

    Plasma::Svg *m_background;
    QPixmap* m_bitmapBackground;
    QString m_wallpaperPath;
};

#endif // PLASMA_PANEL_H
