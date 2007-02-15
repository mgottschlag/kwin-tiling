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

#include "ActionDialog.h"
#include <kiconloader.h>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QMatrix>

namespace KFI
{

// Borrowed from kolourpaint...
static QMatrix matrixWithZeroOrigin(const QMatrix &matrix, int width, int height)
{
    QRect newRect(matrix.mapRect(QRect(0, 0, width, height)));

    return QMatrix(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(),
                   matrix.dx() - newRect.left(), matrix.dy() - newRect.top());
}

static QMatrix rotateMatrix(int width, int height, double angle)
{
    QMatrix matrix;
    matrix.translate(width/2, height/2);
    matrix.rotate(angle);

    return matrixWithZeroOrigin(matrix, width, height);
}

static const int constNumIcons=8;
static int       theUsageCount;
static QPixmap   *theIcons[constNumIcons];

CActionDialog::CActionDialog(QWidget *parent)
             : KDialog(parent)
{
    static const int constIconSize(48);

    setModal(true);
    itsPixmapLabel=new QLabel(this);
    itsPixmapLabel->setMinimumSize(constIconSize, constIconSize);
    itsPixmapLabel->setMaximumSize(constIconSize, constIconSize);
    itsPixmapLabel->setAlignment(Qt::AlignCenter);

    if(0==theUsageCount++)
    {
        QImage img(KIconLoader::global()->loadIcon("font_truetype", K3Icon::NoGroup, 32));
        double increment=360.0/constNumIcons;

        for(int i=0; i<constNumIcons; ++i)
            theIcons[i]=new QPixmap(0==i ? img : img.transformed(rotateMatrix(img.width(), img.height(), increment*i)));
    }

    itsPixmapLabel->setPixmap(*theIcons[0]);
    itsTimer=new QTimer(this);
    connect(itsTimer, SIGNAL(timeout()), SLOT(rotateIcon()));
}

CActionDialog::~CActionDialog()
{
    if(0==--theUsageCount)
        for(int i=0; i<constNumIcons; ++i)
        {
            delete theIcons[i];
            theIcons[i]=NULL;
        }
}

int CActionDialog::exec()
{
    startAnimation();
    return KDialog::exec();
}

void CActionDialog::startAnimation()
{
    itsCount=0;
    itsPixmapLabel->setPixmap(*theIcons[0]);
    itsTimer->start(1000/constNumIcons);
}

void CActionDialog::stopAnimation()
{
    itsTimer->stop();
    itsCount=0;
    itsPixmapLabel->setPixmap(*theIcons[itsCount]);
}

void CActionDialog::rotateIcon()
{
    if(++itsCount==constNumIcons)
        itsCount=0;

    itsPixmapLabel->setPixmap(*theIcons[itsCount]);
}

}

#include "ActionDialog.moc"
