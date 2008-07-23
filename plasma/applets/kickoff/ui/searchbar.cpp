/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "ui/searchbar.h"

// Qt
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QTimer>

// KDE
#include <KIcon>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>

#include "ui/itemdelegate.h"

using namespace Kickoff;

class SearchBar::Private
{
public:
    Private() : editWidget(0),timer(0) {}

    KLineEdit *editWidget;
    QTimer *timer;
};

SearchBar::SearchBar(QWidget *parent)
 : QWidget(parent)
 , d(new Private)
{
    // timer for buffered updates
    d->timer = new QTimer(this);
    d->timer->setInterval(300);
    d->timer->setSingleShot(true);
    connect(d->timer,SIGNAL(timeout()),this,SLOT(updateTimerExpired()));
    connect(this,SIGNAL(startUpdateTimer()),d->timer,SLOT(start()));

    // setup UI
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(3);
    layout->setSpacing(0); // we do the spacing manually to line up with the views below

    QLabel *searchLabel = new QLabel(i18n("Search:"),this);
    QLabel *searchIcon = new QLabel(this);

    QFileInfo fi(QDir(QDir::homePath()), ".face.icon");
    if (fi.exists()) {
        searchIcon->setPixmap(QPixmap(fi.absoluteFilePath()).scaled(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    }
    else {
        searchIcon->setPixmap(KIcon("system-search").pixmap(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    }

    d->editWidget = new KLineEdit(this);
    d->editWidget->installEventFilter(this);
    d->editWidget->setClearButtonShown(true);
    connect(d->editWidget,SIGNAL(textChanged(QString)),this,SIGNAL(startUpdateTimer()));

    //add arbitrary spacing
    layout->addSpacing(2);
    layout->addWidget(searchIcon);
    layout->addSpacing(5);
    layout->addWidget(searchLabel);
    layout->addSpacing(5);
    layout->addWidget(d->editWidget);
    setLayout(layout);

    setFocusProxy(d->editWidget);
}

void SearchBar::updateTimerExpired()
{
    emit queryChanged(d->editWidget->text());
}

SearchBar::~SearchBar()
{
    delete d;
}

bool SearchBar::eventFilter(QObject *watched,QEvent *event)
{
    // left and right arrow key presses in the search edit when the
    // edit is empty are propagated up to the parent widget
    // this allows views in the Launcher to use left and right arrows for
    // navigation whilst the search bar still has the focus
    if (watched == d->editWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) &&
            d->editWidget->text().isEmpty()) {
            QCoreApplication::sendEvent(this,event);
            return true;
        }
    }
    return false; 
}

void SearchBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setPen(QPen(palette().mid(), 1));
    p.drawLine(0, height() - 1, width() - 1, height() - 1);
}

void SearchBar::clear()
{
    d->editWidget->clear();
}

#include "searchbar.moc"
