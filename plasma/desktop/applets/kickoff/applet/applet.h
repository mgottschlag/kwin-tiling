/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef APPLET_H
#define APPLET_H

// KDE

// Plasma
#include <Plasma/PopupApplet>

namespace Kickoff
{
class Launcher;
}
namespace Plasma
{
}

class LauncherApplet : public Plasma::PopupApplet
{
    Q_OBJECT

public:
    LauncherApplet(QObject *parent, const QVariantList &args);
    virtual ~LauncherApplet();

    void init();

    void constraintsEvent(Plasma::Constraints constraints);

    virtual QList<QAction*> contextualActions();

    QWidget *widget();

public slots:
    void switchMenuStyle();
    void startMenuEditor();
    void toolTipAboutToShow();
    void configChanged();

    /**
     * Save config values stored on SimpleLauncher after a menu switch
     */
    void saveConfigurationFromSimpleLauncher(const KConfigGroup & configGroup,
                                             const KConfigGroup & globalConfigGroup);

protected slots:
    void configAccepted();
    //void toggleMenu();
    //void toggleMenu(bool pressed);

protected:

    void createConfigurationInterface(KConfigDialog *parent);
    void popupEvent(bool show);

private:
    friend class Kickoff::Launcher;
    class Private;
    Private * const d;
};

K_EXPORT_PLASMA_APPLET(launcher, LauncherApplet)

#endif
