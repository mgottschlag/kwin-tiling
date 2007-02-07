#ifndef __FONT_PREVIEW_H__
#define __FONT_PREVIEW_H__

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

#include <QString>
#include <QPixmap>
#include <QSize>
#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <kurl.h>
#include "KfiConstants.h"
#include "FcEngine.h"

namespace KFI
{

class CFontPreview : public QWidget
{
    Q_OBJECT

    public:

    CFontPreview(QWidget *parent);
    virtual ~CFontPreview() {}

    void        paintEvent(QPaintEvent *);
    QSize       sizeHint() const;
    QSize       minimumSizeHint() const;

    void        showFont(const KUrl &url, const QString &name=QString::null,
                         unsigned long styleInfo=KFI_NO_STYLE_INFO, int face=1);
    void        showFont();


    public Q_SLOTS:

    void        setUnicodeRange(const QList<CFcEngine::TRange> &r);
    void        showFace(int face);

    Q_SIGNALS:

    void        status(bool);

    private:

    QPixmap                  itsPixmap;
    KUrl                     itsCurrentUrl;
    int                      itsCurrentFace,
                             itsLastWidth,
                             itsLastHeight,
                             itsStyleInfo;
    QString                  itsFontName;
    QList<CFcEngine::TRange> itsRange;
};

}

#endif
