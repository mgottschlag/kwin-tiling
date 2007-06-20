/***************************************************************************
 *   Copyright (C) 2007 by Thomas Georgiou <TAGeorgiou@gmail.com>          *
 *   Copyright (C) 2007 by Jeff Cooper <weirdsox11@gmail.com>              *
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

#include "dict.h"

#include <math.h>

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
//#include <QTextArea>
#include <QLineEdit>
#include <iostream>


#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>
#include <QTime>
#include <plasma/svg.h>


Dict::Dict(QObject *parent, const QStringList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0),
      m_showTimeStringCheckBox(0),
      m_spinSize(0)
{
    setFlags(QGraphicsItem::ItemIsMovable);

    KConfigGroup cg = appletConfig();
	m_layout = new Plasma::VBoxLayout(0);
	m_layout->setGeometry(QRectF(0,0,400,800));
	m_lineEdit = new Plasma::LineEdit(this);
        m_lineEdit->setTextInteractionFlags(Qt::TextEditorInteraction);
	m_layout->addItem(m_lineEdit);
        m_layout->setMargin(12);
	m_word = QString("cuckoldry");
    dataEngine("dict")->connectSource(m_word, this);
    //constraintsUpdated();
}

QRectF Dict::boundingRect() const
{
    return m_layout->geometry();
        //return m_bounds;
//    return m_lineEdit->geometry();
}

void Dict::constraintsUpdated()
{
    prepareGeometryChange();
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        QSize s = QSize(400,400);//m_theme->size();
        m_bounds = QRect(0, 0, s.width(), s.height());
    } else {
        QFontMetrics fm(QApplication::font());
        m_bounds = QRectF(0, 0, fm.width("00:00:00") * 1.2, fm.height() * 1.5);
    }
}

void Dict::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = QTime::currentTime();
	d_thedef = data[QString("gcide")];
	if (m_defDisplay!=0)
		m_defDisplay->setText(d_thedef.toString());
	m_lineEdit->updated(QString("test"),data);
    update();
}

void Dict::configureDialog()
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( "Configure Dict" );

         m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
         connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
         connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
		
	m_defDisplay = new QTextEdit;
	m_defDisplay->setText(d_thedef.toString());
	m_wordChooser = new QLineEdit;
         QWidget* configWidget = new QWidget(m_dialog);
         QVBoxLayout *lay = new QVBoxLayout(configWidget);
	lay->addWidget(m_wordChooser);
         lay->addWidget(m_defDisplay);
         m_dialog->setMainWidget(configWidget);
    }

	m_defDisplay->setText(d_thedef.toString());
    m_dialog->show();
	std::cout<<"SHOW\n";
}

void Dict::configAccepted()
{

	QString m_newWord = m_wordChooser->text();
        dataEngine("dict")->disconnectSource(m_word, this);
        dataEngine("dict")->connectSource(m_newWord, this);
	m_word = m_newWord;
	constraintsUpdated();
}

Dict::~Dict()
{
}


void Dict::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
	Applet::paint(p, option, widget);
        p->drawRoundRect(this->boundingRect(),25);
//     QRectF tempRect(0, 0, 0, 0);
//     QRectF boundRect = boundingRect();
// 
//     QSizeF boundSize = boundRect.size();
//     QSize elementSize;
// 
//     p->setRenderHint(QPainter::SmoothPixmapTransform);
// 
//     qreal seconds = 6.0 * m_time.second() - 180;
//     qreal minutes = 6.0 * m_time.minute() - 180;
//     qreal hours = 30.0 * m_time.hour() - 180 + ((m_time.minute() / 59.0) * 30.0);
// 
//     if (formFactor() == Plasma::Horizontal ||
//         formFactor() == Plasma::Vertical) {
//         QString time = d_thedef.toString();//m_time.toString();
//         QFontMetrics fm(QApplication::font());
//         p->drawText((int)(boundRect.width() * 0.1), (int)(boundRect.height() * 0.25), time);
//         return;
//     }
//     //m_theme->paint(p, boundRect, "DictFace");
// 
// /*    p->save();
//     p->translate(boundSize.width()/2, boundSize.height()/2);
//     p->rotate(hours);
//     elementSize = m_theme->elementSize("HourHand");
// 
//     p->translate(-elementSize.width()/2, -elementSize.width());
//     tempRect.setSize(elementSize);
//     m_theme->paint(p, tempRect, "HourHand");
//     p->restore();
// 
// //     drawHand(p, hours, "SecondHand", 1);
//     p->save();
//     p->translate(boundSize.width()/2, boundSize.height()/2);
//     p->rotate(minutes);
//     elementSize = m_theme->elementSize("MinuteHand");
//     elementSize = QSize(elementSize.width(), elementSize.height());
//     p->translate(-elementSize.width()/2, -elementSize.width());
//     tempRect.setSize(elementSize);
//     m_theme->paint(p, tempRect, "MinuteHand");
//     p->restore();
// 
//     //Make sure we paint the second hand on top of the others
//     if (m_showSecondHand) {
//         p->save();
//         p->translate(boundSize.width()/2, boundSize.height()/2);
//         p->rotate(seconds);
//         elementSize = m_theme->elementSize("SecondHand");
//         elementSize = QSize(elementSize.width(), elementSize.height());
//         p->translate(-elementSize.width()/2, -elementSize.width());
//         tempRect.setSize(elementSize);
//         m_theme->paint(p, tempRect, "SecondHand");
//         p->restore();
//     }
// 
// 
//     p->save();
//     m_theme->resize(boundSize);
//     elementSize = m_theme->elementSize("HandCenterScrew");
//     tempRect.setSize(elementSize);
//     p->translate(boundSize.width() / 2 - elementSize.width() / 2, boundSize.height() / 2 - elementSize.height() / 2);
//     m_theme->paint(p, tempRect, "HandCenterScrew");
//     p->restore();
// 	*/
//     if (m_showTimeString) {
//         //FIXME: temporary time output
//         QString time = d_thedef.toString();
//         QFontMetrics fm(QApplication::font());
//         p->drawText((int)(boundRect.width()/2 - fm.width(time) / 2),
//                     (int)((boundRect.height()/2) - fm.xHeight()*3), d_thedef.toString());
//     }
// 
//     //m_theme->paint(p, boundRect, "Glass");
}

#include "dict.moc"
