////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSettingsWizard
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
 
#include "SettingsWizard.h"
#include "DirSettingsWidget.h"
#include "StarOfficeSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include <klocale.h>
#include <qwizard.h>
#include <qlabel.h>
#include <fstream.h>
#include <stdio.h>

CSettingsWizard::CSettingsWizard(QWidget *parent, const char *name)
               : CSettingsWizardData(parent, name, true)
{
    if(CMisc::root())
    {
        itsNonRootText->hide();
        checkAndModifyFontmapFile();
        checkAndModifyXConfigFile();

        QString fnfTxt=itsFnFText->text();

        itsFnFText->setText(fnfTxt+i18n("\n\nIf \"%1\" is listed as the CUPS folder, it is probable that you are not using the CUPS"
                                        " printing system - in which case just ensure that the chekbox is not selected.").arg(CConfig::constNotFound));
        itsModifiedDirsText->hide();
    }
    else
        if(0==CKfiGlobal::cfg().getModifiedDirs().count())
            itsModifiedDirsText->hide();


    itsSOWidget->hideNote();

#ifndef HAVE_XFT
    removePage(itsAAPage);
#endif

    this->setFinishEnabled(itsCompletePage, true);
}

void CSettingsWizard::checkAndModifyFontmapFile()
{
    //
    // Check if "Fontmap" has been selected by CConfig, and if so, have a look at its contents to see
    // whether it says '(Fontmap.GS) .runlibfile' - if so then use 'Fontmap.GS' instead...
    if(CConfig::constNotFound!=CKfiGlobal::cfg().getGhostscriptFile())
    {
        int slashPos=CKfiGlobal::cfg().getGhostscriptFile().findRev('/');
 
        if(slashPos!=-1)
        {
            QString file=CKfiGlobal::cfg().getGhostscriptFile().mid(slashPos+1);
 
            if("Fontmap"==file)
            {
                ifstream f(CKfiGlobal::cfg().getGhostscriptFile().local8Bit());
 
                if(f)
                {
                    const int constMaxLineLen=1024;
 
                    char line[constMaxLineLen];
                    bool useGS=false;
 
                    do
                    {
                        f.getline(line, constMaxLineLen);
 
                        if(f.good())
                        {
                            line[constMaxLineLen-1]='\0';
 
                            if(strstr(line, "Fontmap.GS")!=NULL && strstr(line, ".runlibfile")!=NULL)
                                useGS=true;
                        }
                    }
                    while(!f.eof() && !useGS);
 
                    f.close();
 
                    if(useGS)
                        itsDirsAndFilesWidget->setGhostscriptFile(CMisc::getDir(CKfiGlobal::cfg().getGhostscriptFile())+"Fontmap.GS");
                }
            }
        }
    }
}

void CSettingsWizard::checkAndModifyXConfigFile()
{
    //
    // Check if XF86Config has been selected by CConfig, and if so, have a look to see wether it has
    // 'unix/<hostname>:<port>' as the fontpath - if so then look for the fontserver 'config' file instead...
    if(CConfig::constNotFound!=CKfiGlobal::cfg().getXConfigFile())
    {
        int slashPos=CKfiGlobal::cfg().getXConfigFile().findRev('/');
 
        if(slashPos!=-1)
        {
            QString file=CKfiGlobal::cfg().getXConfigFile().mid(slashPos+1);
 
            if(file.find("XF86Config")!=-1)
            {
                ifstream f(CKfiGlobal::cfg().getXConfigFile().local8Bit());
 
                if(f)
                {
                    const int constMaxLineLen=1024;
 
                    char line[constMaxLineLen],
                         str1[constMaxLineLen],
                         str2[constMaxLineLen];
                    bool inFiles=false,
                         useXfs=false;
 
                    do
                    {
 
                        f.getline(line, constMaxLineLen);
 
                        if(f.good())
                        {
                            line[constMaxLineLen-1]='\0';
 
                            if('#'!=line[0] && sscanf(line, "%s %s", str1, str2)==2)
                            {
                                if(!inFiles)
                                {
                                    if(strcmp(str1, "Section")==0 && strcmp(str2, "\"Files\"")==0)
                                        inFiles=true;
                                }
                                else
                                    if(strcmp(str1, "FontPath")==0 && str2[0]=='\"')
                                    {
                                        unsigned int s2len=strlen(str2);

                                        if(s2len>8 && str2[s2len-1]=='\"' && &str2[1]==strstr(&str2[1], "unix/") && strchr(&str2[1], ':')!=NULL)
                                            useXfs=true;
                                    }
                            }
                            else
                                if(inFiles && sscanf(line, "%s", str1)==1 && strcmp(str1, "EndSection")==0)
                                    break;
                        }
                    }
                    while(!f.eof() && !useXfs);
 
                    f.close();
 
                    if(useXfs)
                    {
                        int i;
 
                        for(i=0; QString::null!=CConfig::constXfsConfigFiles[i]; ++i)
                            if(CMisc::fExists(CConfig::constXfsConfigFiles[i]))
                            {
                                CKfiGlobal::cfg().setXRefreshCmd(CConfig::XREFRESH_XFS_RESTART);
                                itsDirsAndFilesWidget->setXConfigFile(CConfig::constXfsConfigFiles[i]);
                                break;
                            }
                    }
                }
            }
        }
    }
}
