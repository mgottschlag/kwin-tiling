/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
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

#ifndef PLASMA_PANELVIEW_H
#define PLASMA_PANELVIEW_H

#include <QList>

#include <KConfigGroup>

#include <Plasma/Plasma>
#include <Plasma/View>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

class QWidget;
class QTimeLine;
class QTimer;

class GlowBar;

namespace Plasma
{
    class Containment;
    class Corona;
    class Svg;
}

class PanelController;

class PanelAppletOverlay;

class PanelView : public Plasma::View
{
    Q_OBJECT
public:

    enum VisibilityMode {
        NormalPanel = 0,
        AutoHide,
        LetWindowsCover
    };

   /**
    * Constructs a new panelview.
    * @arg parent the QWidget this panel is parented to
    */
    explicit PanelView(Plasma::Containment *panel, int id = 0, QWidget *parent = 0);
    ~PanelView();

    /**
     * @return the location (screen edge) where this panel is positioned.
     */
    Plasma::Location location() const;

    /**
     * @return panel behaviour
     */
    VisibilityMode visibilityMode() const;

    /**
     * @return the Corona (scene) associated with this panel.
     */
    Plasma::Corona *corona() const;

    /*
     * @return the offset of the panel from the left screen edge
     */
    int offset() const;

    /**
     * @return the panel alignment
     */
    Qt::Alignment alignment() const;

    /**
     * Pinches the min/max sizes of the containment to the current screen resolution
     */
    void pinchContainment(const QRect &screenGeometry);
#ifdef Q_WS_X11
    /**
     * @return the unhide trigger window id, None if there is none
     */
    Window unhideTrigger() { return m_unhideTrigger; }
#endif

    /**
     * Show a visual hint or perhaps even unhide, based on the position of the event
     */
    bool hintOrUnhide(const QPoint &point, bool dueToDnd = false);

    /**
     * Returns the geometry of the current unhide hint window
     */
    QRect unhideHintGeometry() const;

    /**
     * Hides any hide hinting
     */
    void unhintHide();

public Q_SLOTS:
    /**
     * unhides the panel if it is hidden
     */
    void unhide(bool destroyTrigger);

    /**
     * Pinches the min/max sizes of the containment to the current screen resolution
     */
    void pinchContainmentToCurrentScreen();

    /**
     * Sets the offset the left border, the offset is the distance of the left
     * border of the panel from the left border of the screen when the alignment is
     * Qt::AlignLeft, right border and right edge if the alignment is Qt::alignRight
     * and the distance between the center of the panel and the center of the screen if
     * the alignment is Qt::AlignCenter.
     * Similar way for vertical panels.
     * @param newOffset the offset of the panel
     */
    void setOffset(int newOffset);

    /**
     * Sets the edge of the screen the panel will be aligned and will grow
     * @param align the direction (for instance Qt::AlignLeft) means the panel will start
     * from the left of the screen and grow to the right
     */
    void setAlignment(Qt::Alignment align);

    /**
     * Sets the location (screen edge) where this panel is positioned.
     * @param location the location to place the panel at
     */
    void setLocation(Plasma::Location location);

    /**
     * Sets the panel behaviour
     * @param mode
     */
    void setVisibilityMode(PanelView::VisibilityMode mode);

    /**
     * Call when there has been a change that might require changes to the unhide
     * trigger, such as compositing changing.
     */
    void recreateUnhideTrigger();

protected:
    void updateStruts();
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);
    void leaveEvent(QEvent *event);
    void drawBackground(QPainter * painter, const QRectF & rect);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    void createUnhideTrigger();
    void destroyUnhideTrigger();
    bool shouldHintHide() const;
    Qt::Alignment alignmentFilter(Qt::Alignment align) const;
    bool isHorizontal() const;
    QTimeLine *timeLine();
    void positionSpacer(const QPoint pos);
    void startAutoHide();
    bool hasPopup();

#ifdef Q_WS_WIN
    bool registerAccessBar(bool fRegister);
    void appBarQuerySetPos(LPRECT lprc);
    void appBarCallback(WPARAM message, LPARAM lParam);
    void appBarPosChanged();
    bool winEvent(MSG *message, long *result);
    APPBARDATA abd;
#endif

private Q_SLOTS:
    void init();
    void togglePanelController();
    void edittingComplete();
    void animateHide(qreal);
    void panelDeleted();
    void hideMousePoll();
    void unhideHintMousePoll();
    void resetTriggerEnteredSuppression();

    /**
     * Updates the panel's position according to the screen and containment
     * dimensions
     */
    void updatePanelGeometry();

private:
    Plasma::Svg *m_background;
    PanelController *m_panelController;
    QList<PanelAppletOverlay*> m_moveOverlays;
    GlowBar *m_glowBar;
    QTimer *m_mousePollTimer;
    QTimeLine *m_timeLine;
    QGraphicsWidget *m_spacer;
    int m_spacerIndex;

    int m_offset;
    Qt::Alignment m_alignment;
#ifdef Q_WS_X11
    Window m_unhideTrigger;
    QRect m_triggerZone;
    QRect m_unhideTriggerGeom;
#endif

    QSizeF m_lastMin;
    QSizeF m_lastMax;
    VisibilityMode m_visibilityMode;
    int m_lastSeenSize;
    bool m_lastHorizontal : 1;
    bool m_init : 1;

    bool m_editting : 1;
    bool m_firstPaint : 1;
    bool m_triggerEntered : 1;
};

#endif

