/*
 *   Copyright (C) 2005 by Matt Williams <matt@milliams.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "controlbox.h"

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QListView>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimeLine>
#include <KLocale>
#include <KDebug>

ControlBox::ControlBox(QWidget* parent) : QWidget(parent)
{
    displayLabel = new QLabel(i18n("Configure Desktop"), this);
    displayLabel->show();
    resize(displayLabel->size());

    //the config box
    box = new QWidget(parent);
    setupBox();
    box->hide();

    //Set up the animation timeline
    timeLine = new QTimeLine(1000, this);
    timeLine->setFrameRange(0, 50); //50 step anumation
    timeLine->setCurveShape(QTimeLine::EaseOutCurve);
    connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(animateBox(int)));

    connect(this, SIGNAL(boxRequested()), this, SLOT(showBox()));
}

ControlBox::~ControlBox()
{
}

void ControlBox::setupBox()
{
    //This is all to change of course
    QVBoxLayout* boxLayout = new QVBoxLayout(box);

    QPushButton* hideBoxButton = new QPushButton(i18n("Hide Config Box"), box);
    connect(hideBoxButton, SIGNAL(pressed()), this, SLOT(hideBox()));
    QListView* appletList = new QListView(box);
    boxLayout->addWidget(hideBoxButton);
    boxLayout->addWidget(appletList);
    hideBoxButton->show();
    appletList->show();
    box->setLayout(boxLayout);
    box->resize(400,500);
}

void ControlBox::showBox()
{
    box->move(-box->size().width(),-box->size().height());
    box->show();
    timeLine->setDirection(QTimeLine::Forward);
    timeLine->start();
}

void ControlBox::hideBox()
{
    timeLine->setDirection(QTimeLine::Backward);
    timeLine->start();
}

void ControlBox::animateBox(int frame)
{
    //Display the config box
    qreal boxWidth = box->size().width();
    qreal boxHeight = box->size().height();
    qreal boxStep = ((qreal(frame)/50) - 1.0);
    box->move(boxWidth*boxStep,boxHeight*boxStep);

    //And hide the label
    qreal labelWidth = displayLabel->size().width();
    qreal labelHeight = displayLabel->size().height();
    qreal labelStep = (-qreal(frame)/50);
    displayLabel->move(labelWidth*labelStep,labelHeight*labelStep);
}

void ControlBox::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit boxRequested();
    }
}

#include "controlbox.moc"
