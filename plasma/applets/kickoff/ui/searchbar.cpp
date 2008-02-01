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
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

// KDE
#include <KIcon>
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

    QFileInfo fi(QDir(qgetenv("HOME")), ".face.icon");
    if (fi.exists()) {
        searchIcon->setPixmap(QPixmap(fi.absoluteFilePath()).scaled(ItemDelegate::ICON_SIZE, ItemDelegate::ICON_SIZE));
    }
    else {
        searchIcon->setPixmap(KIcon("system-search").pixmap(ItemDelegate::ICON_SIZE, ItemDelegate::ICON_SIZE));
    }

    d->editWidget = new KLineEdit(this);
    d->editWidget->installEventFilter(this);
    d->editWidget->setClearButtonShown(true);
    connect(d->editWidget,SIGNAL(textChanged(QString)),this,SIGNAL(startUpdateTimer()));

    layout->addSpacing(ItemDelegate::ITEM_LEFT_MARGIN - 3);
    layout->addWidget(searchIcon);
    layout->addSpacing(ItemDelegate::ICON_TEXT_MARGIN);
    layout->addWidget(searchLabel);
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

void SearchBar::clear()
{
    d->editWidget->clear();
}

#include "searchbar.moc"
