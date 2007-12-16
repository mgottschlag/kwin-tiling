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

#ifndef LAUNCHER_H
#define LAUNCHER_H

// Qt
#include <QWidget>

namespace Kickoff
{

/** 
 * The main window class for the Kickoff launcher.  This class is responsible
 * for creating the various tabs, views and models which make up the launcher's
 * user interface.
 */
class Launcher : public QWidget
{
Q_OBJECT

public:
    /** Construct a new Launcher with the specified parent. */
    Launcher(QWidget *parent = 0);
    ~Launcher();

    /** Specifies whether the launcher should hide itself when an item is activated. */
    void setAutoHide(bool autoHide);
    bool autoHide() const;

    // reimplemented
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual QSize sizeHint() const;

signals:
    void aboutToHide();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void moveEvent(QMoveEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private Q_SLOTS:
    void focusSearchView(const QString& query);
    void focusFavoritesView();
    void showViewContextMenu(const QPoint& pos);

private:
    class Private;
    Private * const d;
};

}

#endif // LAUNCHER_H
