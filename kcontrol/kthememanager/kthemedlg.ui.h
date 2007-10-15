/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#ifndef KTHEMEDLG_UI_H
#define KTHEMEDLG_UI_H

#include <krun.h>

void KThemeDlg::startKonqui( const QString & url )
{
    (void) new KRun(url,this);
}

void KThemeDlg::startBackground()
{
    KRun::runCommand("kcmshell4 background", this);
}

void KThemeDlg::startColors()
{
    KRun::runCommand("kcmshell4 colors", this);
}

void KThemeDlg::startStyle()
{
    KRun::runCommand("kcmshell4 style", this);
}

void KThemeDlg::startIcons()
{
    KRun::runCommand("kcmshell4 icons", this);
}

void KThemeDlg::startFonts()
{
   KRun::runCommand("kcmshell4 fonts", this);
}

void KThemeDlg::startSaver()
{
    KRun::runCommand("kcmshell4 screensaver", this);
}

#endif // KTHEMEDLG_UI_H
