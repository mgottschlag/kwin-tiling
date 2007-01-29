/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef APPLETITEMDELEGATE_H
#define APPLETITEMDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>
#include <QPixmap>
#include <QPainter>
#include <kicon.h>

class AppletItemDelegate
    : public QItemDelegate
{
    Q_OBJECT
    Q_ENUMS(AppletRole)

public:
    /**
      * @brief Constructor for the item delegate.
      */
    AppletItemDelegate(QObject *parent = 0);

    /**
      * @brief Destructor.
      */
    ~AppletItemDelegate();


    /**
      * @brief Paints the item delegate.
      */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /**
      * @brief Gets the size of the item delegate.
      */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    /**
      * Sets the icon size to @p width x @p height
      * All icons will have @p width x @p height size
      * @param width the width of the icon
      * @param height the height of the icon
      */
    void setIconSize(int width, int height);

    /**
      * Sets the minimum row width to @p minimumItemWidth
      * @param minimumItemWidth the minimum width you want to set the delegate to
      */
    void setMinimumItemWidth(int minimumItemWidth);

    /**
      * Sets the left margin of the delegate to @p leftMargin
      * @param leftMargin the left margin that will be set
      */
    void setLeftMargin(int leftMargin);

    /**
      * Sets the right margin of the delegate to @p rightMargin
      * @param rightMargin the right margin that will be set
      */
    void setRightMargin(int rightMargin);

    /**
      * Sets the number of pixels that will be drawn blank on top and bottom
      * of the row to @p separatorPixels
      * @param separatorPixels the number of pixels you wanto to set as separator
      */
    void setSeparatorPixels(int separatorPixels);

    enum AppletRole
    {
        SecondaryDisplayRole = 33
    };

private:
    class Private;
    Private *d;
};

#endif
