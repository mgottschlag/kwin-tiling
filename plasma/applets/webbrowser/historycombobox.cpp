/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2008 Marco Martin <notmart@gmail.com>
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

#include "historycombobox.h"

#include <KHistoryComboBox>


namespace Plasma
{

class HistoryComboBox::Private
{
public:
    Private()
    {
    }

    ~Private()
    {
        historyCombo = 0;
    }

    KHistoryComboBox *historyCombo;
};

HistoryComboBox::HistoryComboBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new Private)
{
    KHistoryComboBox* native = new KHistoryComboBox;

    connect(native, SIGNAL(cleared()), this, SIGNAL(cleared()));
    connect(native, SIGNAL(returnPressed(const QString &)), this, SIGNAL(returnPressed(const QString &)));
    connect(native, SIGNAL(textChanged(const QString &)), this, SIGNAL(textChanged(const QString &)));
    connect(native, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
    connect(native, SIGNAL(activated(int)), this, SIGNAL(activated(int)));
    setWidget(native);

    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    d->historyCombo = native;
}

HistoryComboBox::~HistoryComboBox()
{
    delete d;
}


void HistoryComboBox::setStylesheet(const QString &stylesheet)
{
    d->historyCombo->setStyleSheet(stylesheet);
}

QString HistoryComboBox::stylesheet()
{
    return d->historyCombo->styleSheet();
}

KHistoryComboBox* HistoryComboBox::nativeWidget() const
{
    return static_cast<KHistoryComboBox*>(d->historyCombo);
}

void HistoryComboBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

QStringList HistoryComboBox::historyItems() const
{
    return d->historyCombo->historyItems();
}

bool HistoryComboBox::removeFromHistory(const QString &item)
{
    return d->historyCombo->removeFromHistory(item);
}

void HistoryComboBox::reset()
{
    d->historyCombo->reset();
}

void HistoryComboBox::setHistoryItems(const QStringList &items)
{
    d->historyCombo->setHistoryItems(items);
}

QString HistoryComboBox::currentText() const
{
    return d->historyCombo->currentText();
}

void HistoryComboBox::insertUrl(int index, const KUrl &url)
{
    d->historyCombo->insertUrl(index, url);
}

void HistoryComboBox::setDuplicatesEnabled(bool enabled)
{
    d->historyCombo->setDuplicatesEnabled(enabled);
}

void HistoryComboBox::addToHistory(const QString &item)
{
    d->historyCombo->addToHistory(item);
}

}// namespace Plasma

#include <historycombobox.moc>

