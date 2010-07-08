/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2010 by Davide Bettio <davide.bettio@kdemail.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef SELECTIONTOGGLE_H
#define SELECTIONTOGGLE_H

#include <kurl.h>

#include <QAbstractButton>
#include <QPixmap>

class QTimeLine;

/**
 * @brief Toggle button for changing the selection of an hovered item.
 *
 * The removeButton button is visually invisible until it is displayed at least
 * for one second.
 *
 * @see RemoveButtonManager
 */
class RemoveButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit RemoveButton(QWidget* parent);
    virtual ~RemoveButton();
    virtual QSize sizeHint() const;

    /**
     * Resets the selection removeButton so that it is hidden and stays
     * visually invisible for at least one second after it is shown again.
     */
    void reset();
    void setItemName(const QString& name);
    QString itemName() const;

public slots:
    virtual void setVisible(bool visible);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private slots:
    /**
     * Sets the alpha value for the fading animation and is
     * connected with m_fadingTimeLine.
     */
    void setFadingValue(int value);

    void refreshIcon();

private:
    void startFading();
    void stopFading();

private:
    bool m_isHovered;
    bool m_leftMouseButtonPressed;
    int m_fadingValue;
    QPixmap m_icon;
    QTimeLine* m_fadingTimeLine;
    QString m_itemName;
};

#endif
