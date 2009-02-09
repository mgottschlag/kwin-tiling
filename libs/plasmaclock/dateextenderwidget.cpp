/*
 *   Copyright 2009 Davide Bettio <davide.bettio@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "dateextenderwidget.h"

#include <QDate>
#include <QGraphicsLinearLayout>

#include <Plasma/Label>
#include <Plasma/DataEngine>

DateExtenderWidget::DateExtenderWidget(QDate date, Plasma::DataEngine *engine, QGraphicsWidget *parent)
    : QGraphicsWidget(parent)
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    Plasma::Label *descriptionLabel = new Plasma::Label();

    QString tmpStr = "description:it:" + date.toString(Qt::ISODate);
    descriptionLabel->setText(i18n("Holiday: %1", engine->query(tmpStr).value(tmpStr).toString()));
    layout->addItem(descriptionLabel);
}

DateExtenderWidget::~DateExtenderWidget()
{

}

#include "dateextenderwidget.moc"
