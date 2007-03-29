#ifndef __FONT_VIEW_PART_H__
#define __FONT_VIEW_PART_H__

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

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kconfig.h>
#include <kurl.h>
#include <QFrame>
#include <QMap>
#include "KfiConstants.h"
#include "FontPreview.h"

class QPushButton;
class QLabel;
class QProcess;
class QAction;
class KIntNumInput;
class KConfig;

namespace KFI
{

class BrowserExtension;

class CFontViewPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

    public:

    CFontViewPart(QWidget *parent=0);
    virtual ~CFontViewPart();

    bool openUrl(const KUrl &url);

    protected:

    bool openFile();

    public Q_SLOTS:

    void previewStatus(bool st);
    void timeout();
    void install();
    void installlStatus();
    void changeText();
    void print();
    void displayType(const QList<CFcEngine::TRange> &range);
    void showFace(int f);
    void statResult(KJob *job);

    Q_SIGNALS:

    void enablePrintAction(bool enable);

    private:

    void stat(const QString &path=QString());
    void getMetaInfo(int face);

    private:

    QMap<int, QString> itsMetaInfo;
    CFontPreview       *itsPreview;
    QPushButton        *itsInstallButton;
    QWidget            *itsFaceWidget;
    QFrame             *itsFrame;
    QLabel             *itsFaceLabel,
                       *itsMetaLabel;
    KIntNumInput       *itsFaceSelector;
    QAction            *itsChangeTextAction;
    int                itsFace;
    KSharedConfigPtr   itsConfig;
    BrowserExtension   *itsExtension;
    QProcess           *itsProc;
    QString            itsStatName;
    KUrl               itsMetaUrl;
};

class BrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

    public:

    BrowserExtension(CFontViewPart *parent);

    void enablePrint(bool enable);

    public Q_SLOTS:

    void print();
};

}

#endif
