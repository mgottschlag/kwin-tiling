/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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
#ifndef KCMGESTURES_H
#define KCMGESTURES_H

#include <KCModule>

#include <QtCore/QModelIndex>

class KCMGesturesPrivate;

class QWidget;

/**
 * @brief KCMGestures KDE KCM Hotkeys Configuration Module
 * @author Michael Jansen <kde@michael-jansen.biz>
 * @date 2008-03-07
 */
class KCMGestures : public KCModule
    {
    Q_OBJECT

public:

    /**
     * Create the module.
     *
     * @param parent Parent widget
     */
    KCMGestures( QWidget *parent, const QVariantList &arg );

    /**
     * Destroy the module
     */
    virtual ~KCMGestures();

    /**
     * Set all settings back to defaults.
     */
    void defaults();

    /**
     * Load all settings. 
     */
    void load();

    /**
     * Save the settings
     */
    void save();



public Q_SLOTS:

    void slotChanged();

    /**
     * Call when the current item has changed
     */
    void currentChanged( const QModelIndex &current, const QModelIndex &previous );

private:
    KCMGesturesPrivate *d;
};

#endif /* #ifndef KCMGESTURES_HPP */
