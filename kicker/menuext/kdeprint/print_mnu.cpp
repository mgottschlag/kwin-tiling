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

#include "print_mnu.h"
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <krun.h>
#include <kdeprint/kmmanager.h>
#include <QRegExp>
//Added by qt3to4:
#include <Q3PtrList>
#include <ktoolinvocation.h>

K_EXPORT_KICKER_MENUEXT(kdeprint, PrintMenu)

#define ADD_PRINTER_ID		0
#define KDEPRINT_SETTINGS_ID	1
#define	CONFIG_SERVER_ID	2
#define	PRINT_MANAGER_ID	3
#define PRINT_BROWSER_ID	4
#define	KPRINTER_ID		5
#define PRINTER_LIST_ID		10

PrintMenu::PrintMenu(QWidget *parent, const QStringList & /*args*/)
: KPanelMenu("", parent)
{
    static bool kdeprintIconsInitialized = false;
    if ( !kdeprintIconsInitialized ) {
        KIconLoader::global()->addAppDir("kdeprint");
        kdeprintIconsInitialized = true;
    }
}

PrintMenu::~PrintMenu()
{
}

void PrintMenu::initialize()
{
    if (initialized()) clear();
    setInitialized(true);

    int ID = PRINTER_LIST_ID;
    // just to be sure the plugin is loaded -> icons are available
    KMManager::self();

    if ((KMManager::self()->printerOperationMask() & KMManager::PrinterCreation) && KMManager::self()->hasManagement())
        insertItem(KIcon("wizard"), i18n("Add Printer..."), ADD_PRINTER_ID);
    insertItem(KIcon("kdeprint_configmgr"), i18n("KDE Print Settings"), KDEPRINT_SETTINGS_ID);
    if (KMManager::self()->serverOperationMask() & KMManager::ServerConfigure)
        insertItem(KIcon("kdeprint_configsrv"), i18n("Configure Server"), CONFIG_SERVER_ID);
    addSeparator();
    insertItem(KIcon("kcontrol"), i18n("Print Manager"), PRINT_MANAGER_ID);
    insertItem(KIcon("konqueror"), i18n("Print Browser (Konqueror)"), PRINT_BROWSER_ID);
    addSeparator();
    insertItem(KIcon("fileprint"), i18n("Print File..."), KPRINTER_ID);

    // printer list
    QList<KMPrinter*>    l = KMManager::self()->printerList();
    if (!l.isEmpty())
    {
        bool separatorInserted = false;
		QListIterator<KMPrinter*>       it(l);
		while (it.hasNext())
        {
			KMPrinter *itprt = it.next();
            // no special, implicit or pure instances
            if (itprt->isSpecial() || itprt->isVirtual())
                continue;
            if (!separatorInserted)
            {
                // we insert a separator only when we find the first
                // printer
                addSeparator();
                separatorInserted = true;
            }
            insertItem(KIcon(itprt->pixmap()),
                       itprt->printerName(), ID++);
        }
    }
}

void PrintMenu::slotExec(int ID)
{
    switch (ID)
    {
        case ADD_PRINTER_ID:
            KToolInvocation::kdeinitExec("kaddprinterwizard");
            break;
        case KDEPRINT_SETTINGS_ID:
	    KToolInvocation::kdeinitExec("kaddprinterwizard", QStringList("--kdeconfig"));
            break;
	case CONFIG_SERVER_ID:
	    KToolInvocation::kdeinitExec("kaddprinterwizard", QStringList("--serverconfig"));
	    break;
        case PRINT_MANAGER_ID:
            KRun::runCommand("kcmshell kde-printers.desktop");
            break;
        case PRINT_BROWSER_ID:
            KRun::runCommand("kfmclient openProfile filemanagement print:/", "kfmclient", "konqueror");
            break;
	case KPRINTER_ID:
	    KToolInvocation::kdeinitExec("kprinter");
	    break;
        default:
            {
                // start kjobviewer
                QStringList args;
                args << "--show" << "-d" << text(ID).remove('&');
                KToolInvocation::kdeinitExec("kjobviewer", args);
            }
            break;
    }
}

void PrintMenu::reload()
{
	initialize();
}

#include "print_mnu.moc"
