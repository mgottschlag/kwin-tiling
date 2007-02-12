#ifndef __FONT_FILTER_H__
#define __FONT_FILTER_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <klineedit.h>
#include <QPixmap>

class QLabel;
class QMenu;
class QActionGroup;
class KToggleAction;

namespace KFI
{

class CFontFilter : public KLineEdit
{
    Q_OBJECT

    public:

    enum ECriteria
    {
        CRIT_FAMILY,
        CRIT_STYLE,
        CRIT_FILENAME,
        CRIT_LOCATION,

        NUM_CRIT
    };

    CFontFilter(QWidget *parent);
    virtual ~CFontFilter() { }

    void setMgtMode(bool m);

    Q_SIGNALS:

    void criteriaChanged(int crit);

    private Q_SLOTS:

    void filterChanged();

    private:

    void addAction(ECriteria crit, const QString &text, bool on, bool visible);
    void paintEvent(QPaintEvent *ev);
    void resizeEvent(QResizeEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void setCriteria(ECriteria crit);

    private:

    QLabel        *itsMenuButton;
    QMenu         *itsMenu;
    ECriteria     itsCurrentCriteria;
    QPixmap       itsPixmaps[NUM_CRIT];
    KToggleAction *itsActions[NUM_CRIT];
    QActionGroup  *itsActionGroup;
};

}

#endif
