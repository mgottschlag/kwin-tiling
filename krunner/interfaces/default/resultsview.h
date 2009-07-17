/***************************************************************************
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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


#ifndef ResultView_H
#define ResultView_H

#include <KDebug>

namespace Plasma
{
    class Svg;
}
class ResultScene;
class QGraphicsView;
class QToolButton;

class ResultsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ResultsView ( QWidget *parent = 0);
    ~ResultsView();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

    Plasma::Svg *m_arrowSvg;

    private slots:
    void updateArrowsVisibility();
    void previousPage();
    void nextPage();

private:
    void resetArrowsPosition();

    QToolButton *m_previousPage;
    QToolButton *m_nextPage;
    QPixmap m_previousFadeout;
    QPixmap m_nextFadeout;
};

#endif
