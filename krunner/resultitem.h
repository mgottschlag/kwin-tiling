/***************************************************************************
 *   Copyright 2007 by Enrico Ros <enrico.ros@gmail.com>                   *
 *   Copyright 2007 by Riccardo Iaconelli <ruphy@kde.org>                  *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef __ResultItem_h__
#define __ResultItem_h__

#include <QtGui/QGraphicsWidget>
#include <QtGui/QIcon>
#include <plasma/querymatch.h>

class QGraphicsLinearLayout;

class ResultItemSignaller : public QObject
{
    Q_OBJECT

public:
    ResultItemSignaller(QObject *parent = 0)
        : QObject(parent)
    {

    }

    void startAnimations()
    {
        emit animate();
    }

Q_SIGNALS:
    void animate();
};

class ResultItem : public QGraphicsWidget
{
    Q_OBJECT

public:
    ResultItem(const Plasma::QueryMatch &match, QGraphicsWidget *parent);

    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    void setMatch(const Plasma::QueryMatch &match);

    // getters
    QString id() const;
    int updateId() const;
    QString name() const;
    QString description() const;
    QString data() const;
    QIcon icon() const;
    Plasma::QueryMatch::Type group() const;
    uint priority() const;
    bool isFavorite() const;
    bool isDefault() const;
    void setIndex(int index);
    int index() const;
    void setRowStride(int stride);
    int rowStride() const;
    void remove();

    void setUpdateId(int id);
    void setIsDefault(bool isDefault);

    static const int ITEM_SIZE = 68;
    static const int PADDING = 2;
    static const int MARGIN = 3;
    static const int BOUNDING_SIZE = ITEM_SIZE + MARGIN*2;

signals:
    void indexReleased(int index);
    void activated(ResultItem *item);
    void hoverEnter(ResultItem *item);
    void hoverLeave(ResultItem *item);

protected:
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *e);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *e);
    void timerEvent(QTimerEvent *e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    // must always call remove()
    ~ResultItem();
    class Private;
    Private * const d;
//    Q_PRIVATE_SLOT(d, void animationComplete())

private slots:
    void slotTestTransp();
    void animationComplete();
    void animate();
    void becomeVisible();
};

#endif
