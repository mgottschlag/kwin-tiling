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

#ifndef DICT_H
#define DICT_H

#include <QImage>
#include <QPaintDevice>
#include <QLabel>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <QTime>
#include <QX11Info>
#include <QWidget>
#include <QGraphicsItem>
#include <QColor>
#include <QTextEdit>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/vboxlayout.h>
#include <plasma/lineedit.h>

class QTimer;
class QCheckBox;
class QSpinBox;
class QLineEdit;


class KDialog;
class KTimeZoneWidget;

namespace Plasma
{
    class Svg;
}

class Dict : public Plasma::Applet
{
    Q_OBJECT
    public:
        Dict(QObject *parent, const QStringList &args);
        ~Dict();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget =0);
        void setPath(const QString&);
        QRectF boundingRect() const;
        void constraintsUpdated();

    public slots:
        void updated(const QString &name, const Plasma::DataEngine::Data &data);
        void configureDialog();

    protected slots:
//         void acceptedTimeStringState(bool);
        void configAccepted();

    private:

        bool m_showTimeString;
        bool m_showSecondHand;
        QRectF m_bounds;
        int m_pixelSize;
        QString m_timezone;
        Plasma::Svg* m_theme;
        QTime m_time;
	QVariant d_thedef;
        KDialog *m_dialog; //should we move this into another class?
        QCheckBox *m_showTimeStringCheckBox;
        QCheckBox *m_showSecondHandCheckBox;
        QCheckBox *m_swapTzs;
        QSpinBox *m_spinSize;
	QTextEdit *m_defDisplay;
	QLineEdit *m_wordChooser;
	QString m_word;
       
	Plasma::VBoxLayout *m_layout;
	Plasma::LineEdit *m_lineEdit;
};

K_EXPORT_PLASMA_APPLET(dict, Dict)

#endif
