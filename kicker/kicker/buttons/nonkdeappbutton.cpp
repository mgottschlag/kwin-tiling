/*****************************************************************

Copyright (c) 1996-2001 the kicker authors. See file AUTHORS.

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

#include <QToolTip>
#include <Qt3Support/Q3ColorDrag>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

#include <kconfig.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <krun.h>
#include <k3process.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconeffect.h>
#include <kdebug.h>
#include <kworkspace.h>

// the header where the configuration dialog is defined.
#include "exe_dlg.h"

// our own definition
#include "nonkdeappbutton.h"
#include <kconfiggroup.h>

// we include the "moc" file so that the KDE build system knows to create it
#include "nonkdeappbutton.moc"

// this is one of the two constructors. gets called when creating a new button
// e.g. via the "non-KDE Application" dialog, not one that was saved and then
// restored.
NonKDEAppButton::NonKDEAppButton(const QString& name,
                                 const QString& description,
                                 const QString& filePath, const QString& icon,
                                 const QString &cmdLine, bool inTerm,
                                 QWidget* parent)
  : PanelButton(parent) // call our superclass's constructor
{
    // call the initialization method
    initialize(name, description, filePath, icon, cmdLine, inTerm);

    // and connect the clicked() signal (emitted when the button is activated)
    // to the slotExec() slot
    // we do this here instead of in initialize(...) since initialize(...) may
    // get called later, e.g after reconfiguring it
    connect(this, SIGNAL(clicked()), SLOT(slotExec()));
}

// this constructor is used when restoring a button, usually at startup
NonKDEAppButton::NonKDEAppButton( const KConfigGroup& config, QWidget* parent )
  : PanelButton(parent) // call our superclass's constructor
{
    // call the initialization method, this time with values from a config file
    initialize(config.readEntry("Name"),
               config.readEntry("Description"),
               config.readPathEntry("Path"),
               config.readEntry("Icon"),
               config.readPathEntry("CommandLine"),
               config.readEntry("RunInTerminal", false));

    // see comment on connect in above constructor
    connect(this, SIGNAL(clicked()), SLOT(slotExec()));
}

void NonKDEAppButton::initialize(const QString& name,
                                 const QString& description,
                                 const QString& filePath, const QString& icon,
                                 const QString &cmdLine, bool inTerm )
{
    setObjectName("NonKDEAppButton");
    // and now we actually set up most of the member variables with the
    // values passed in here. by doing this all in an initialize() method
    // we avoid duplicating this code all over the place
    nameStr = name;
    descStr = description;
    pathStr = filePath;
    iconStr = icon;
    cmdStr = cmdLine;
    term = inTerm;

    // now we set the buttons tooltip, title and icon using the appropriate
    // set*() methods from the PanelButton class from which we subclass

    // assign the name or the description to a QString called tooltip
    QString tooltip = description.isEmpty() ? nameStr : descStr;

    if (tooltip.isEmpty())
    {
        // we had nothing, so let's try the path
        tooltip = pathStr;

        // and add the command if we have one.
        if (!cmdStr.isEmpty())
        {
            tooltip += ' ' + cmdStr;
        }

        // set the title to the pathStr
        setTitle(pathStr);
    }
    else
    {
        // since we have a name or a description (assigned by the user) let's
        // use that as the title
        setTitle(nameStr.isEmpty() ? descStr : nameStr);
    }

    // set the tooltip
    this->setToolTip( tooltip);

    // set the icon
    setIcon(iconStr);
}

void NonKDEAppButton::saveConfig( KConfigGroup& config ) const
{
    // this is called whenever we change something
    // the config object sent in will already be set to the
    // right group and file, so we can just start writing
    config.writeEntry("Name", nameStr);
    config.writeEntry("Description", descStr);
    config.writeEntry("RunInTerminal", term);
    config.writePathEntry("Path", pathStr);
    config.writeEntry("Icon", iconStr);
    config.writePathEntry("CommandLine", cmdStr);
}

void NonKDEAppButton::dragEnterEvent(QDragEnterEvent *ev)
{
    // when something is dragged onto this button, we'll accept it
    // if we aren't dragged onto ourselves, and if it's a URL
    if ((ev->source() != this) && KUrl::List::canDecode(ev->mimeData()))
    {
        ev->accept(rect());
    }
    else
    {
        ev->ignore(rect());
    }

    // and now let the PanelButton do as it wishes with it...
    PanelButton::dragEnterEvent(ev);
}

void NonKDEAppButton::dropEvent(QDropEvent *ev)
{
    // something has been droped on us!
    KUrl::List fileList = KUrl::List::fromMimeData(ev->mimeData());
    QString execStr;

    // according to KUrl, we've successfully retrieved
    // one or more URLs! now we iterate over them one by
    // one ....
    for (KUrl::List::ConstIterator it = fileList.begin();
         it != fileList.end();
         ++it)
    {
        const KUrl &url(*it);
        if (KDesktopFile::isDesktopFile(url.path()))
        {
            // this URL is actually a .desktop file, so let's grab
            // the URL it actually points to ...
            KDesktopFile deskFile(url.path());
            // ... and add it to the exec string
            execStr += K3Process::quote(deskFile.readUrl()) + ' ';
        }
        else
        {
            // it's just a URL of some sort, add it directly to the exec
            execStr += K3Process::quote(url.path()) + ' ';
        }
    }


    // and now run the command
    if (!execStr.isEmpty())
    {
        runCommand(execStr);
    }


    // and let PanelButton clean up
    PanelButton::dropEvent(ev);
}

void NonKDEAppButton::slotExec()
{
    // the button was clicked, let's take some action
    runCommand();
}

void NonKDEAppButton::runCommand(const QString& execStr)
{
    // run our command! this method is used both by the drag 'n drop
    // facilities as well as when the button is activated (usualy w/a click)

    // we'll use the "result" variable to record our success/failure
    bool result;

    // since kicker doesn't listen to or use the session manager, we have
    // to make sure that our environment is set up correctly. this is
    // accomlplished by doing:
    KWorkSpace::propagateSessionManager();

    if (term)
    {
        // run in a terminal? ok! we find this in the config file in the
        // [misc] group (this will usually be in kdeglobal, actually, which
        // get merged into the application config automagically for us
        KConfigGroup config(KGlobal::config(), "misc");
        QString termStr = config.readPathEntry("Terminal", "konsole");

        // and now we run the darn thing and store how we fared in result
        result = KRun::runCommand(termStr + " -e " + pathStr + ' ' +
                                  cmdStr + ' ' + execStr,
                                  pathStr, iconStr);
    }
    else
    {
        // just run it...
        result = KRun::runCommand(pathStr + ' ' + cmdStr  + ' ' + execStr,
                                  pathStr, iconStr);
    }

    if (!result)
    {
        // something went wrong, so let's show an error msg to the user
        KMessageBox::error(this, i18n("Cannot execute non-KDE application."),
                           i18n("Kicker Error"));
        return;
    }
}

void NonKDEAppButton::updateSettings(PanelExeDialog* dlg)
{
    // we were reconfigured via the confiugration dialog
    // re-setup our member variables using initialize(...),
    // this time using values from the dlg
    initialize(dlg->title(), dlg->description(), dlg->command(),
               dlg->iconPath(), dlg->commandLine(), dlg->useTerminal());

    // now delete the dialog so that it doesn't leak memory
    delete dlg;

    // and emit a signal that the container that houses us
    // listens for so it knows when to start the process to
    // save our configuration
    emit requestSave();
}

void NonKDEAppButton::properties()
{
    // the user has requested to configure this button
    // this method gets called by the ButtonContainer that houses the button
    // see ButtonContainer::eventFilter(QObject *o, QEvent *e), where the
    // context menu is created and dealt with.

    // so we create a new config dialog ....
    PanelExeDialog* dlg = new PanelExeDialog(nameStr, descStr, pathStr,
                                             iconStr, cmdStr, term, this);

    // ... connect the signal it emits when it has data for us to save
    // to our updateSettings slot (see above) ...
    connect(dlg, SIGNAL(updateSettings(PanelExeDialog*)), this,
            SLOT(updateSettings(PanelExeDialog*)));

    // ... and then show it to the user
    dlg->show();
}

