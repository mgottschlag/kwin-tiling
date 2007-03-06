/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Daniel Molkentin <molkentin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtGui>
#include <QTimeLine>
#include <collapsiblewidget.h>

/******************************************************************
 * Private classes
 *****************************************************************/

class CollapsibleWidget::Private
{
    public:
        bool            expanded;
        QGridLayout    *gridLayout;
        QWidget        *innerWidget;
        QTimeLine      *timeline;
};

class SettingsContainer::Private
{
    public:
        QVBoxLayout *layout;
};

/******************************************************************
 * Implementation
 *****************************************************************/

SettingsContainer::SettingsContainer(QWidget *parent)
: QScrollArea( parent ), d(new SettingsContainer::Private)
{
    QWidget *w = new QWidget;
    QVBoxLayout *helperLay = new QVBoxLayout(w);
    d->layout = new QVBoxLayout;
    helperLay->addLayout( d->layout );
    helperLay->addStretch(1);
    setWidget(w);
    setWidgetResizable(true);
}

SettingsContainer::~SettingsContainer()
{
    delete d;
}

CollapsibleWidget* SettingsContainer::insertWidget( QWidget *w, const QString& name )
{
    Q_UNUSED( name )

    if ( w && w->layout() ) {
        QLayout *lay = w->layout();
        lay->setMargin(2);
        lay->setSpacing(0);
    }

    CollapsibleWidget *cw = new CollapsibleWidget( this );
    d->layout->addWidget( cw );
    cw->setInnerWidget( w );
    return cw;
}

CollapsibleWidget::CollapsibleWidget(QWidget *parent)
: QWidget(parent), d(new CollapsibleWidget::Private)
{
    init();
}

void CollapsibleWidget::init()
{
    d->expanded = false;
    d->innerWidget = 0;

    d->timeline = new QTimeLine( 150, this );
    d->timeline->setCurveShape( QTimeLine::EaseInOutCurve );
    connect( d->timeline, SIGNAL(valueChanged(qreal)),
            this, SLOT(animateCollapse(qreal)) );
    connect( d->timeline, SIGNAL(finished()),
            this, SLOT(signalCompletion()) );

    d->gridLayout = new QGridLayout( this );
    d->gridLayout->setMargin(0);

    setExpanded(false);
    setEnabled(false);
}

CollapsibleWidget::~CollapsibleWidget()
{
    delete d;
}

QWidget* CollapsibleWidget::innerWidget() const
{
    return d->innerWidget;
}

void CollapsibleWidget::setInnerWidget(QWidget *w)
{
    if (!w) {
        return;
    }

    d->innerWidget = w;

    if ( !isExpanded() ) {
        d->innerWidget->hide();
    }
    d->gridLayout->addWidget( d->innerWidget, 2, 2 );
    d->gridLayout->setRowStretch( 2, 1 );

    setEnabled( true );

    if ( isExpanded() ) {
        setExpanded( true );
    }
}

void CollapsibleWidget::setExpanded(bool expanded)
{
    if ( !d->innerWidget ) {
        return;
    }

    d->expanded = expanded;

    if ( !expanded ) {
        d->innerWidget->setVisible( false );
    }

    d->timeline->setDirection( expanded ? QTimeLine::Forward
                                        : QTimeLine::Backward );
    d->timeline->start();
}

void CollapsibleWidget::animateCollapse( qreal showAmount )
{
    int pixels = d->innerWidget->sizeHint().height() * showAmount;

    d->gridLayout->setRowMinimumHeight( 2, pixels );

    if ( showAmount == 1 ) {
        d->innerWidget->setVisible( true );
    }
}

void CollapsibleWidget::signalCompletion()
{
    if ( isExpanded() ) {
        emit expandCompleted();
    } else {
        emit collapseCompleted();
    }
}

bool CollapsibleWidget::isExpanded() const
{
    return d->expanded;
}

#include "collapsiblewidget.moc"
