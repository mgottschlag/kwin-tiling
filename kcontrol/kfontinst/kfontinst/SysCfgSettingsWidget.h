#ifndef __SYS_CFG_SETTINGS_WIDGET_H__
#define __SYS_CFG_SETTINGS_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSysCfgSettingsWidget
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

#include "SysCfgSettingsWidgetData.h"

class CSysCfgSettingsWidget : public CSysCfgSettingsWidgetData
{
    Q_OBJECT

    public:

    CSysCfgSettingsWidget(QWidget *parent, const char *name=NULL);
    virtual ~CSysCfgSettingsWidget() {}

    void encodingSelected(bool on);
    void generateAfmsSelected(bool on);
    void customXRefreshSelected(bool on);
    void xfsRestartSelected(bool on);
    void xsetFpRehashSelected(bool on);
    void customXStrChanged(const QString &str);
    void encodingSelected(const QString &str);
    void afmEncodingSelected(const QString &str);
    void t1AfmSelected(bool on);
    void ttAfmSelected(bool on);
    void overwriteAfmsSelected(bool on);

    signals:

    void afmGenerationDeselected();

    public slots:

    void scanEncodings();
    void enableAfmGeneration();
};

#endif
