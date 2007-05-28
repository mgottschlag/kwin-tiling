/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
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

#include "ciavc.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>

#include <dataenginemanager.h>
#include <svg.h>
#include <widgets/vboxlayout.h>
#include <widgets/lineedit.h>

CiaVc::CiaVc(QObject *parent, const QStringList &args)
    : Plasma::Applet(parent, args) //"plasma-clock-default", appletId)
{
    setFlags(QGraphicsItem::ItemIsMovable); // | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    m_engine = Plasma::DataEngineManager::self()->loadDataEngine("ciavc");

    m_layout = new Plasma::VBoxLayout(0);
    m_layout->setGeometry(QRectF(0, 0, 400, 800));
    m_layout->setMargin(12);

    if (m_engine) {
        foreach (const QString& source, m_engine->dataSources()) {
            sourceAdded(source);
        }
        connect(m_engine, SIGNAL(newDataSource(QString)), this, SLOT(sourceAdded(QString)));
        connect(m_engine, SIGNAL(dataSourceRemoved(QString)), this, SLOT(sourceRemoved(QString)));
    }
}

CiaVc::~CiaVc()
{
    Plasma::DataEngineManager::self()->unloadDataEngine("ciavc");
    delete m_layout;
}

QRectF CiaVc::boundingRect() const
{
    return m_layout->geometry();
}

void CiaVc::sourceAdded(const QString& source)
{
    Plasma::LineEdit* text = new Plasma::LineEdit(this);
    text->setObjectName(source);
    kDebug() << "added " << text->objectName() << " with expandings " << text->expandingDirections() << endl;
    m_engine->connectSource(source, text);
    m_layout->addItem(text);
}

void CiaVc::sourceRemoved(const QString& source)
{
    foreach (QGraphicsItem * child, QGraphicsItem::children()) {
        Plasma::LineEdit * text = dynamic_cast<Plasma::LineEdit*>(child);
        if (!text) {
            continue;
        }

        if (text->objectName() == source) {
            delete text;
        }
        return;
    }
}

void CiaVc::configureDialog()
{
    //TODO: implement configuration of projects
}

void CiaVc::acceptedConfigDialog()
{
    KConfigGroup cg = globalAppletConfig();
    //TODO: implement configuration of projects
}

void CiaVc::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    Applet::paint(p, option, widget);
}

#include "ciavc.moc"
