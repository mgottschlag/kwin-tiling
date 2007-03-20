/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __nonkdeappbutton_h__
#define __nonkdeappbutton_h__

// pull in the superclass's definition
#include "panelbutton.h"
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

// forward declare this class
// lets the compiler know it exists without have to know all the gory details
class PanelExeDialog;

/**
 * Button that contains a non-KDE application
 * subclass of PanelButton
 */
class NonKDEAppButton : public PanelButton
{
    // the Q_OBJECT macro provides the magic glue for signals 'n slots
    Q_OBJECT

public:
    // define our two constructors, one used for creating new buttons...
    NonKDEAppButton(const QString& name, const QString& description,
                    const QString& filePath, const QString& icon,
                    const QString& cmdLine, bool inTerm, QWidget* parent);

    // ... and once for restoring them at start up
    NonKDEAppButton(const KConfigGroup& config, QWidget* parent);

    // reimplemented from PanelButton
    virtual void saveConfig(KConfigGroup& config) const;
    virtual void properties();

protected Q_SLOTS:
    // called when the button is activated
    void slotExec();

    // called after the user reconfigures something
    void updateSettings(PanelExeDialog* dlg);

protected:
    // used to set up our internal state, either when creating the button
    // or after reconfiguration
    void initialize(const QString& name, const QString& description,
                    const QString& filePath, const QString& icon,
                    const QString& cmdLine, bool inTerm);

    // run the command!
    // the execStr parameter, which default to an empty string,
    // is used to provide additional command line options aside
    // from the ones in our config file; for instance a URL drag'd onto us
    void runCommand(const QString& execStr = QString());

    // reimplemented from PanelButton
    QString defaultIcon() const { return "exec"; }

    // handle drag and drop actions
    virtual void dropEvent(QDropEvent *ev);
    virtual void dragEnterEvent(QDragEnterEvent *ev);

    QString    nameStr; // the name given this button by the user
    QString    descStr; // the description given this button by the user
    QString    pathStr; // the path to the command
    QString    iconStr; // the path to the icon for this button
    QString    cmdStr;  // command line flags, if any
    bool       term;    // whether to run this in a terminal or not
}; // all done defining the class!

#endif
