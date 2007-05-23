/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
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

#include "rootwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <KWindowSystem>

#include "svg.h"
#include "desktop.h"
#include "controlbox.h"

RootWidget::RootWidget()
    : QWidget(0, Qt::FramelessWindowHint)
{
    setFocusPolicy( Qt::NoFocus );
    setGeometry( QApplication::desktop()->geometry() );
    lower();

    KWindowSystem::setType(winId(), NET::Desktop);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setMargin(0);
    rootLayout->setSpacing(0);

    m_desktop = new Desktop(this);
    rootLayout->addWidget(m_desktop);
    m_desktop->show();

    connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize()));
    m_controlBox = new ControlBox(this);
    m_controlBox->show();
}

RootWidget::~RootWidget()
{
}

Desktop* RootWidget::desktop() const
{
    return m_desktop;
}

void RootWidget::adjustSize()
{
    setGeometry( QApplication::desktop()->geometry() );
}

#include "rootwidget.moc"

