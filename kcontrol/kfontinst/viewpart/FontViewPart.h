#ifndef __FONT_VIEW_PART_H__
#define __FONT_VIEW_PART_H__

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

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <QtGui/QFrame>
#include "KfiConstants.h"
#include "FontPreview.h"

class QPushButton;
class QLabel;
class QBoxLayout;
class KIntNumInput;
class QAction;
class KSelectAction;
class KUrl;
class KConfig;
class KProcess;

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
    void installlStatus(KProcess *proc);
    void changeText();
    void print();
    void displayType();

    Q_SIGNALS:

    void enablePrintAction(bool enable);

    private:

    void doPreview(bool isFonts, const QString &name=QString::null,
                   unsigned long styleInfo=KFI_NO_STYLE_INFO, int index=1);
    bool isInstalled();

    private:

    CFontPreview     *itsPreview;
    QPushButton      *itsInstallButton;
    QBoxLayout       *itsLayout;
    QFrame           *itsFrame,
                     *itsToolsFrame;
    QLabel           *itsFaceLabel;
    KIntNumInput     *itsFaceSelector;
    QAction          *itsChangeTextAction;
    KSelectAction    *itsDisplayTypeAction;
    int              itsFace;
    KConfig          *itsConfig;
    BrowserExtension *itsExtension;
    KProcess         *itsProc;
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
