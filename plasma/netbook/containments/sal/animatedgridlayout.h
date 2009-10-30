/***********************************************************************/
/* animatedgridlayout.h                                                */
/*                                                                     */
/* Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>*/
/* Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>*/
/*                                                                     */
/* This library is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU Lesser General Public          */
/* License as published by the Free Software Foundation; either        */
/* version 2.1 of the License, or (at your option) any later version.  */
/*                                                                     */
/* This library is distributed in the hope that it will be useful,     */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/* Lesser General Public License for more details.                     */
/*                                                                     */
/* You should have received a copy of the GNU Lesser General Public    */
/* License along with this library; if not, write to the Free Software */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       */
/* 02110-1301  USA                                                     */
/***********************************************************************/
#ifndef ANIMATEDLINEARLAYOUT_H
#define ANIMATEDLINEARLAYOUT_H

#include <QGraphicsGridLayout>
#include <QObject>

class AnimatedGridLayout :public QGraphicsGridLayout
{
public:
    AnimatedGridLayout(QGraphicsLayoutItem *parent = 0);

    void addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment = 0);
    QGraphicsLayoutItem *itemAt(int index) const;
    QGraphicsLayoutItem *itemAt(int row, int column) const;
    void removeAt(int index);
};

#endif
