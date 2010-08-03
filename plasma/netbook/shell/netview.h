/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
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

#ifndef NETVIEW_H
#define NETVIEW_H

#include <Plasma/Plasma>
#include <Plasma/View>

class NetPanelController;
class QPropertyAnimation;

namespace Plasma
{
    class Containment;
} // namespace Plasma

class NetView : public Plasma::View
{
    Q_OBJECT

public:
    typedef Plasma::ImmutabilityType ImmutabilityType;
    NetView(Plasma::Containment *containment, int uid, QWidget *parent = 0);
    ~NetView();

    /**
     * hook up all needed signals to a containment
     */
    void connectContainment(Plasma::Containment *containment);

    Plasma::Location location() const;
    Plasma::FormFactor formFactor() const;
    KConfigGroup config() const {return Plasma::View::config();}
    bool autoHide() const;

    static int mainViewId() { return 1; }
    static int controlBarId() { return 2; }

    void setUseGL(const bool on);
    bool useGL() const;


public Q_SLOTS:
    void setContainment(Plasma::Containment *containment);
    void screenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment* containment);
    void updateGeometry();
    void grabContainment();
    void updateConfigurationMode(bool config);
    void setAutoHide(bool autoHide);
    void immutabilityChanged(ImmutabilityType immutability);
    void nextContainment();
    void previousContainment();

Q_SIGNALS:
    void locationChanged(const NetView *view);
    void geometryChanged();
    void containmentActivated();
    void autoHideChanged(bool autoHide);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent *e);

private:
    NetPanelController *m_panelController;
    bool m_configurationMode;
    bool m_useGL;
    QPropertyAnimation *m_containmentSwitchAnimation;
};

#endif // multiple inclusion guard
