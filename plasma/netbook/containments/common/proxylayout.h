/***********************************************************************/
/* proxylayout.h                                                       */
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
#ifndef PROXYLAYOUT_H
#define PROXYLAYOUT_H

#include <QGraphicsLayout>
#include <QObject>

class ProxyLayoutPrivate;

class ProxyLayout :public QGraphicsLayoutItem
{
public:
    ProxyLayout(QGraphicsWidget *widget, QGraphicsLayoutItem *parent = 0);
    ~ProxyLayout();

    QGraphicsLayoutItem *widget();
    void setGeometry(const QRectF &rect);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint=QSizeF()) const;

private:
    ProxyLayoutPrivate  * const d;
};

#endif
