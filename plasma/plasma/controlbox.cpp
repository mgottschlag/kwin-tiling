/*
 *   Copyright (C) 2007 by Matt Williams <matt@milliams.com>
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
#include <QtGui/QStandardItemModel>
#include <QtCore/QStringList>
#include <KLocale>
#include <KDebug>
#include <KLibrary>
//#include <KLibLoader>

#include "applet.h"
#include "svg.h"

//BEGIN - DisplayLabel

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

//BEGIN- PlasmoidListItemModel

PlasmoidListItemModel::PlasmoidListItemModel(QWidget* parent)
    : QStandardItemModel(parent)
{
}

QStringList PlasmoidListItemModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("text/x-plasmoidservicename");
    return types;
}

QMimeData* PlasmoidListItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0)
        return 0;
    QStringList types = mimeTypes();
    if (types.isEmpty())
        return 0;
    QMimeData *data = new QMimeData();
    QString format = types.at(0);
    QByteArray byteArray;
    QStandardItem* selectedItem = item(indexes[0].row(), 1);
    byteArray.append(selectedItem->data(Qt::DisplayRole).toByteArray());
    data->setData(format, byteArray);
    return data;
}

//BEGIN - ControlWidget

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget(parent)
{
    //This is all to change of course
    QVBoxLayout* boxLayout = new QVBoxLayout(this);

    QPushButton* hideBoxButton = new QPushButton(i18n("Hide Config Box"), this);
    m_appletList = new QListView(this);
    m_appletList->setDragEnabled(true);
    m_label = new QLabel("Plasmoids:", this);
    boxLayout->addWidget(hideBoxButton);
    boxLayout->addWidget(m_label);
    boxLayout->addWidget(m_appletList);

    hideBoxButton->show();
    m_appletList->show();
    setLayout(boxLayout);

    connect(hideBoxButton, SIGNAL(pressed()), parent, SLOT(hideBox()));

    m_appletListModel = new PlasmoidListItemModel(this);
    m_appletList->setModel(m_appletListModel);
    m_appletList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_appletList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(addPlasmoidSlot(QModelIndex)));

    //TODO: this should be delayed until (if) the box is actually shown.
    refreshPlasmoidList();
}

ControlWidget::~ControlWidget() {}

void ControlWidget::refreshPlasmoidList()
{
    KPluginInfo::List applets = Plasma::Applet::knownApplets();

    m_appletListModel->clear();
    m_appletListModel->setColumnCount(2);
    m_appletListModel->setRowCount(applets.count());

    int count = 0;
    foreach (KPluginInfo* info, applets) {
        m_appletListModel->setItem(count, 0, new QStandardItem(info->name()));
        m_appletListModel->setItem(count, 1, new QStandardItem(info->pluginName()));
        ++count;
    }
}

void ControlWidget::addPlasmoidSlot(const QModelIndex& plasmoidIndex)
{
    emit addPlasmoid(m_appletListModel->item(plasmoidIndex.row(), 1)->text());
}

//BEGIN - ControlBox

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
    m_box = new ControlWidget(this);
    m_box->installEventFilter(this);
    m_box->hide();
    boxIsShown = false;
    connect(m_box, SIGNAL(addPlasmoid(const QString&)), this, SIGNAL(addPlasmoid(const QString&)));

    //Set up the animation timeline
    m_timeLine = new QTimeLine(300, this);
    m_timeLine->setFrameRange(0, 25); //25 step anumation
    m_timeLine->setCurveShape(QTimeLine::EaseInOutCurve);
    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(animateBox(int)));
    connect(m_timeLine, SIGNAL(finished()), this, SLOT(finishBoxHiding()));

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

void ControlBox::showBox()
{
    if(boxIsShown) {
        return;
    }
    boxIsShown = true;
    m_box->move(-m_box->size().width(),-m_box->size().height());
    resize(m_box->sizeHint()); //resize this widget so the full contents of m_box can be seen.
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

void ControlBox::finishBoxHiding()
{
    if (m_timeLine->direction() == QTimeLine::Backward) {
        resize(m_displayLabel->size()); //resize this widget so it's only the size of the label
        m_box->hide();
    }
}

/*void ControlBox::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit boxRequested();
    }
}*/

#include "controlbox.moc"
