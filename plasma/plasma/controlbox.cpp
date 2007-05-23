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
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtCore/QTimeLine>
#include <QtCore/QTimer>
#include <KLocale>
#include <KDebug>

#include "svg.h"

class DisplayLabel : public QLabel
{
    public:
        DisplayLabel(const QString& text, QWidget *parent);
        //QSize minimumSizeHint();

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        Plasma::Svg m_background;
};

DisplayLabel::DisplayLabel(const QString& text, QWidget *parent)
    : QLabel(text, parent),
      m_background("background/dialog") //FIXME: we need a proper SVG for this =)
{
    setAlignment(Qt::AlignCenter);
//    resize(minimumSizeHint());
    m_background.resize(size());
}

/*
QSize DisplayLabel::minimumSizeHint()
{
    QSize size = QLabel::minimumSizeHint();
    size.setHeight(size.height() * 2);
    size.setWidth(size.width() * 5 / 3);
    return size;
}
*/

void DisplayLabel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter p(this);
    m_background.paint(&p, rect());
    QLabel::paintEvent(event);
}

ControlBox::ControlBox(QWidget* parent) : QWidget(parent)
{
    //The display box label/button
    m_displayLabel = new DisplayLabel(i18n("Configure Desktop"), this);
    m_displayLabel->show();
    m_displayLabel->installEventFilter(this);
    resize(m_displayLabel->size());

    //The hide box timer
    m_exitTimer = new QTimer(this);
    m_exitTimer->setInterval(300);
    m_exitTimer->setSingleShot(true);
    connect(m_exitTimer, SIGNAL(timeout()), this, SLOT(hideBox()));

    //the config box
    m_box = new QWidget(this);
    m_box->installEventFilter(this);
    setupBox();
    m_box->hide();
    boxIsShown = false;

    //Set up the animation timeline
    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0, 25); //25 step anumation
    m_timeLine->setCurveShape(QTimeLine::EaseInOutCurve);
    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(animateBox(int)));

    connect(this, SIGNAL(boxRequested()), this, SLOT(showBox()));
}

ControlBox::~ControlBox()
{
}

bool ControlBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_displayLabel) {
        if (event->type() == QEvent::Enter) {
            showBox();
        }
    } else if (watched == m_box) {
        if (event->type() == QEvent::Leave) {
            m_exitTimer->start();
        } else if (event->type() == QEvent::Enter) {
            m_exitTimer->stop(); //If not a leave event, stop the box from closing
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ControlBox::setupBox()
{
    //This is all to change of course
    QVBoxLayout* boxLayout = new QVBoxLayout(m_box);

    QPushButton* hideBoxButton = new QPushButton(i18n("Hide Config Box"), m_box);
    connect(hideBoxButton, SIGNAL(pressed()), this, SLOT(hideBox()));
    QListView* appletList = new QListView(m_box);
    boxLayout->addWidget(hideBoxButton);
    boxLayout->addWidget(appletList);
    hideBoxButton->show();
    appletList->show();
    m_box->setLayout(boxLayout);
    m_box->resize(400,500);
}

void ControlBox::showBox()
{
    if(boxIsShown) {
        return;
    }
    boxIsShown = true;
    m_box->move(-m_box->size().width(),-m_box->size().height());
    resize(m_box->size()); //resize this widget so the full contents of m_box can be seen.
    m_box->show();
    m_timeLine->setDirection(QTimeLine::Forward);
    m_timeLine->start();
}

void ControlBox::hideBox()
{
    if(!boxIsShown) {
        return;
    }
    boxIsShown = false;
    m_timeLine->setDirection(QTimeLine::Backward);
    m_timeLine->start();
}

void ControlBox::animateBox(int frame)
{
    if ((frame == 1) && (m_timeLine->direction() == QTimeLine::Backward)) {
        resize(m_displayLabel->size()); //resize this widget so it's only the size of the label
        m_box->hide();
    }

    //Display the config box
    qreal boxWidth = m_box->size().width();
    qreal boxHeight = m_box->size().height();
    qreal boxStep = ((qreal(frame)/25) - 1.0);
    m_box->move(static_cast<int>(boxWidth*boxStep),static_cast<int>(boxHeight*boxStep));

    //And hide the label
    qreal labelWidth = m_displayLabel->size().width();
    qreal labelHeight = m_displayLabel->size().height();
    qreal labelStep = (-qreal(frame)/25);
    m_displayLabel->move(static_cast<int>(labelWidth*labelStep),static_cast<int>(labelHeight*labelStep));
}

void ControlBox::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit boxRequested();
    }
}

#include "controlbox.moc"
