#ifndef __FONTS_WIDGET_H__
#define __FONTS_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "FontListWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "UiConfig.h"
#include <qnamespace.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qwidget.h>

class CSysConfigurer;
class CFontPreview;
class CFontSelectorWidget;
class CMetaDialog;
class KProgress;
class QPushButton;
class QGroupBox;
class QComboBox;
class QLabel;
class QSplitter;

class CFontsWidget : public QWidget
{
    Q_OBJECT

    public:

    CFontsWidget(QWidget *parent, const char *);
    virtual ~CFontsWidget();

    void reset()    { itsFontList->reset(); }
    void scanDirs();
    void storeSettings();

    signals:

    void progressActive(bool);
    void configuredSystem();
    void madeChanges();

    public slots:

    void initProgress(const QString &title, int numSteps);
    void progress(const QString &str);
    void stopProgress();
    void configureSystem();
    void systemConfigured();
    void flMadeChanges();
    void installFs();
    void setMode(int mode);
    void showMetaData(QStringList files);

    private:

    void setMode(CUiConfig::EMode mode, bool canShowFsDirs);

    private:

    CFontListWidget     *itsFontList;
    KProgress           *itsProgress;
    QLabel              *itsLabel;
    CFontPreview        *itsPreview;
    QGroupBox           *itsProgressBox,
                        *itsSelectorBox;
    QPushButton         *itsButtonAdd,
                        *itsButtonRemove,
                        *itsButtonDisable,
                        *itsButtonEnable;
    QComboBox           *itsModeCombo;
    CSysConfigurer      *itsSysConfigurer;
    QLabel              *itsFontListLabel;
    CFontSelectorWidget *itsSelector;
    CMetaDialog         *itsMetaDialog;
    QSplitter           *itsMainSplitter;
};

#endif
