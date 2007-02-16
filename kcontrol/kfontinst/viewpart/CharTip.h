#ifndef __CHAR_TIP_H__
#define __CHAR_TIP_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

#include <QFrame>
#include <QPixmap>
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>
#include "FcEngine.h"

class QLabel;
class QTimer;

namespace KFI
{

class CFontPreview;

class CCharTip : public QFrame
{
    Q_OBJECT

    public:

    CCharTip(CFontPreview *parent);
    ~CCharTip();

    void setItem(const CFcEngine::TChar &ch);

    private Q_SLOTS:

    void showTip();
    void hideTip();

    private:

    void reposition();
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *, QEvent *e);

    private:

    CFontPreview     *itsParent;
    QLabel           *itsLabel,
                     *itsPixmapLabel;
    QTimer           *itsTimer;
    CFcEngine::TChar itsItem;
};

}

#endif
