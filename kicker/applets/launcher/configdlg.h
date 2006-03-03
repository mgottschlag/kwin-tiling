/*****************************************************************

Copyright (c) 2005 Fred Schaettgen <kde.sch@ttgen.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef CONFIG_DLG_H
#define CONFIG_DLG_H

#include <kconfigdialog.h>

class ConfigDlgBase;
class Prefs;

class ConfigDlg : public KConfigDialog
{
    Q_OBJECT

public:
    ConfigDlg(QWidget *parent, const char *name, Prefs *config, int autoSize,
        KConfigDialog::DialogType dialogType, ButtonCodes dialogButtons);

protected:
    virtual bool hasChanged();

protected Q_SLOTS:
    virtual void updateSettings();
    virtual void updateWidgets();
    virtual void updateWidgetsDefault();

private:
    ConfigDlgBase *m_ui;
    Prefs* m_settings;
    int m_autoSize;
    QString m_oldIconDimText;
};

#endif
