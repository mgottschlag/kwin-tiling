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

#include "FontsWidgetData.h"
#include "InstalledFontListWidget.h"
#include <qnamespace.h>
#include <qstring.h>
#include <qpixmap.h>

class CDiskFontListWidget;
class CSysConfigurer;

class CFontsWidget : public CFontsWidgetData
{
    Q_OBJECT

    public:

    CFontsWidget(QWidget *parent, const char *);
    virtual ~CFontsWidget();

    void setPreviewMode(bool on);
    void rescan();
    void setOrientation(Qt::Orientation o);
    void scanDirs();

    signals:

    void progressActive(bool);
    void configuredSystem();

    public slots:

    void preview(const QString &) { }
    void initProgress(const QString &title, int numSteps);
    void progress(const QString &str);
    void stopProgress();
    void configureSystem();
    void systemConfigured();
    void preview(const QString &dir, const QString &file);
    void enableCfgButton() { itsInstalled->enableCfgButton(); }
    void setCfgButton()    { itsInstalled->setCfgButton(); }

    private:

    CDiskFontListWidget      *itsDisk;
    CInstalledFontListWidget *itsInstalled;
    CSysConfigurer           *itsSysConfigurer;
    QString                  itsPreviousTitle,
                             itsPreviousStr;
    QPixmap                  itsPreviousPixmap;
};

#endif
