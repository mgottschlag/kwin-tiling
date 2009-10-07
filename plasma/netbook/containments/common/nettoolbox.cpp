/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "nettoolbox.h"

#include <QGraphicsLinearLayout>

#include <KIconLoader>

#include <Plasma/Frame>
#include <Plasma/Containment>
#include <Plasma/IconWidget>

NebToolBox::NebToolBox(Plasma::Containment *parent)
   : QGraphicsWidget(parent)
{
    m_toolContainer = new Plasma::Frame(this);
    m_toolContainer->setFlag(QGraphicsItem::ItemStacksBehindParent);
    m_toolContainerLayout = new QGraphicsLinearLayout(m_toolContainer);
    resize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
}

NebToolBox::~NebToolBox()
{
}

bool NebToolBox::showing() const
{
    return m_toolContainer->isVisible();
}

void NebToolBox::setShowing(const bool show)
{
    if (show != m_toolContainer->isVisible()) {
        emit toggled();
        emit visibilityChanged(show);
    }
    m_toolContainer->setVisible(show);
}


void NebToolBox::addTool(QAction *action)
{
    Plasma::IconWidget *button = new Plasma::IconWidget(this);
    button->setAction(action);
    m_actionButtons[action] = button;
    m_toolContainerLayout->addItem(button);
}

void NebToolBox::removeTool(QAction *action)
{
    if (m_actionButtons.contains(action)) {
        Plasma::IconWidget *button = m_actionButtons.value(action);
        m_toolContainerLayout->removeItem(button);
        m_actionButtons.remove(action);
        button->deleteLater();
    }
}


#include "nettoolbox.moc"
