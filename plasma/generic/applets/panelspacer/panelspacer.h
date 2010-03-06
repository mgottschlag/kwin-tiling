/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef PANELSPACER_H
#define PANELSPACER_H

#include <Plasma/Applet>


class PanelSpacer : public Plasma::Applet
{
    Q_OBJECT
public:
    PanelSpacer(QObject *parent, const QVariantList &args);
    ~PanelSpacer();
    void init();
    void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect);
    void constraintsEvent(Plasma::Constraints constraints);
    QList<QAction*> contextualActions();

public slots:
    void configChanged();

private Q_SLOTS:
    void updateConfigurationMode(bool config);
    void toggleFixed(bool flexible);

private:
    bool m_configurationMode;
    bool m_fixedSize;
    QList <QAction*> m_actions;
};

#endif

