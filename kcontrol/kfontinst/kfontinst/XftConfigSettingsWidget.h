#ifndef __XFT_CONFIG_SETTINGS_WIDGET_H__
#define __XFT_CONFIG_SETTINGS_WIDGET_H__
 
////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXftConfigSettingsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 28/05/2001
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
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "XftConfigSettingsWidgetData.h"

#ifdef HAVE_XFT
#include <qpushbutton.h>

class CXftConfigRules;
#endif

//
// Can't #ifdef this class out completely, as it's used in the SettingsWizard.ui file
//
class CXftConfigSettingsWidget : public CXftConfigSettingsWidgetData
{
    Q_OBJECT

    public:

    CXftConfigSettingsWidget(QWidget *parent, const char *name=NULL)
#ifdef HAVE_XFT
        ;
#else
        {}
#endif
    virtual ~CXftConfigSettingsWidget() {}

#ifdef HAVE_XFT
    void fileButtonPressed();
    void excludeRangeChecked(bool on);
    void fromChanged(const QString &str);
    void toChanged(const QString &str);
    void useSubPixelChecked(bool on);
    void advancedButtonPressed();
    void saveButtonPressed();

    public slots:

    void disableSaveButton() { itsSaveButton->setEnabled(false); }

    signals:

    void madeChanges();
    void savedChanges();

    private:

    void setWidgets();

    private:

    CXftConfigRules *itsRulesDialog;
#endif
};

#endif
