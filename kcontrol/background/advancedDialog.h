/*
 *  advancedDialog.h
 *
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef __ADVANCEDDIALOG_H
#define __ADVANCEDDIALOG_H

#include <kdialogbase.h>

class QCheckBox;
class QLabel;
class QSpinBox;

class KGlobalBackgroundSettings;
class AdvancedOptions;

class AdvancedDialog : public KDialogBase
{
    Q_OBJECT

    public:
        AdvancedDialog(KGlobalBackgroundSettings* settings,
                       QWidget* parent = 0, char* name = 0);
        ~AdvancedDialog();

    protected slots:
        void load();
        void save();
        void changed();
        void defaults();

    protected:
        KGlobalBackgroundSettings* m_pGlobals;
        AdvancedOptions* m_advancedWidget;
};

#endif
