////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CStarOfficeConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 04/05/2001
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

#include "StarOfficeConfig.h"
#include "Misc.h"
#include "AfmCreator.h"
#include "FontEngine.h"
#include "Encodings.h"
#include "BufferedFile.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <qstring.h>
#include <qcstring.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <klocale.h>

static const QString  constAfmDir       ("fontmetrics/afm/");
static const QCString constSOGuardStr   (" # kfontinst ");
static const QString  constBackupProlog ("xprinter.prolog.kfontinst_backup");

static QString xp3Directory()
{
    if(CMisc::dExists(CKfiGlobal::cfg().getSODir() + "xp3/"))
        return CKfiGlobal::cfg().getSODir() + "xp3/";
    else
        if(CMisc::dExists(CKfiGlobal::cfg().getSODir() + "share/xp3/"))
            return CKfiGlobal::cfg().getSODir() + "share/xp3/";
        else
            return QString::null;
}

CStarOfficeConfig::EStatus CStarOfficeConfig::go(const QString &path)
{
    EStatus status=SUCCESS;
    QDir    dir(path);

    if(dir.isReadable())
    {
        QString       xp3Dir(xp3Directory()),
                      ppdFileName(xp3Dir+"ppds/"+CKfiGlobal::cfg().getSOPpd());
        CBufferedFile ppdFile(ppdFileName.local8Bit(), CBufferedFile::createGuard(constSOGuardStr, path.local8Bit()), "*Font ");

        if(ppdFile)
        {
            QString       psStdFontsFileName(xp3Dir+"psstd.fonts");
            CBufferedFile psStdFontsFile(psStdFontsFileName.local8Bit(), CBufferedFile::createGuard(constSOGuardStr, path.local8Bit(), false), NULL, true, true);

            if(psStdFontsFile)
            {
                const QFileInfoList *files=dir.entryInfoList();

                if(files)
                {
                    QFileInfoListIterator it(*files);
                    QFileInfo             *fInfo;

                    for(; NULL!=(fInfo=it.current()); ++it)
                        if("."!=fInfo->fileName() && ".."!=fInfo->fileName() && !fInfo->isDir() &&
                           SUCCESS==status &&
                           (CFontEngine::isAType1(fInfo->fileName().local8Bit()) || CFontEngine::isATtf(fInfo->fileName().local8Bit())))
                        {
                            QString afmName=CMisc::afmName(fInfo->fileName());

                            emit step(i18n("Adding %1 to StarOffice").arg(fInfo->filePath()));

                            if(CMisc::fExists(path+afmName))  // Does the .afm exists in the X11 dir?
                            {
                                QString soAfm=getAfmName(fInfo->filePath()),
                                        cmd;
                                cmd="\\cd "+xp3Dir+constAfmDir;

                                if(CMisc::fExists(xp3Dir+constAfmDir+soAfm))   // Remove the old one
                                    CMisc::removeFile(xp3Dir+constAfmDir+soAfm);
                                CMisc::linkFile(path+afmName, xp3Dir+constAfmDir+soAfm);

                                if(CMisc::fExists(xp3Dir+constAfmDir+soAfm))
                                {
                                     QCString ppdEntry("*Font ");

                                     soAfm.remove(soAfm.length()-4, 4);
                                     ppdEntry+=soAfm.local8Bit();
                                     ppdEntry+=": Standard \"(001.002)\" Standard ROM";
                                     ppdFile.write(ppdEntry);
                                     status=outputToPsStdFonts(path, psStdFontsFile, fInfo->fileName(), soAfm);
                                }
                            }
                        }
                }

                // No try to use the correct xprinter.prolog...
                bool useStdProlog=false,
                     backupExists;

                emit step(i18n("Setting up xprinter.prolog"));

                backupExists=CMisc::fExists(xp3Dir+constBackupProlog);

                QString encFile(CMisc::locate("StarOffice/"+CKfiGlobal::cfg().getAfmEncoding()+".xpp"));

                if(QString::null!=encFile && CMisc::fExists(encFile))
                {
                    // Remove existing xprinter.prolog, or move to back-up
                    status= (backupExists ? CMisc::removeFile(xp3Dir+"xprinter.prolog") : CMisc::moveFile(xp3Dir+"xprinter.prolog", xp3Dir+constBackupProlog))
                                        ? SUCCESS : COULD_NOT_MODIFY_XPRINTER_DOT_PROLOG;

                    if(SUCCESS==status)  // Copy from encoding.xpp to xprinter.prolog
                        status=CMisc::linkFile(encFile, xp3Dir+"/xprinter.prolog") ? SUCCESS : COULD_NOT_LINK_XPRINTER_DOT_PROLOG;
                }
                else  // No .xpp exists for selected encoding...
                    useStdProlog=true;

                if(useStdProlog)
                    if(backupExists)  // Then xprinter.prolog was for an old encoding, move backup back to xprinter.prolog
                        status=CMisc::removeFile(xp3Dir+"xprinter.prolog") && CMisc::moveFile(xp3Dir+constBackupProlog, xp3Dir+"xprinter.prolog")
                                            ? SUCCESS : COULD_NOT_RESTORE_XPRINTER_DOT_PROLOG;
            }
            else
                status=COULD_NOT_OPEN_PSSTD_FONTS;

            ppdFile.close();
        }
        else
            status=COULD_NOT_OPEN_PPD_FILE;
   }
   else
        status=COULD_NOT_OPEN_X11_DIR;

   return status;
}

void CStarOfficeConfig::removeAfm(const QString &fname)
{
    QString xp3Dir(xp3Directory()),
            afm=getAfmName(fname);
 
    if(CMisc::fExists(xp3Dir+constAfmDir+afm))
        CMisc::removeFile(xp3Dir+constAfmDir+afm);
}

CStarOfficeConfig::EStatus CStarOfficeConfig::outputToPsStdFonts(const QString &xDir, CBufferedFile &out, const QString &fileName, const QString &afm)
{
    ifstream in(QString(xDir+"fonts.dir").local8Bit());
    EStatus  status=SUCCESS;

    if(in)
    {
        const int          constMaxLine=256;
        const unsigned int consMaxPsstdfontsEntryLen=126;
        const char *       searchStr="--0-0-0-0-";

        char line[constMaxLine],
             *pos=NULL;

        do
        {
            in.getline(line, constMaxLine);

            if(!in.eof() && strstr(line, fileName.local8Bit())==line && (pos=strstr(line, searchStr)))
            { 
                *pos='\0';
                pos+=strlen(searchStr);
                QCString entry(afm.local8Bit());

                entry+=", ";
                entry+=&(line[fileName.length()+1]);
                entry+="--%d-%d-%d-%d-";
                entry+=pos;

                if(entry.length()<=consMaxPsstdfontsEntryLen)
                    out.write(entry);
            }
        }
        while(!in.eof());
        in.close();
    }
    else
        status=COULD_NOT_OPEN_FONTS_DOT_SCALE;

    return status;
}

QString CStarOfficeConfig::getAfmName(const QString &file)
{
    QCString afm(CMisc::shortName(file).local8Bit());

    afm.replace(QRegExp("/"), "");

    if(!CMisc::root())
    {
        char *user=getlogin();

        if(!user)
            user=getenv("LOGNAME");
        if(!user)
            user=getenv("USER");

        if(user)
        {
            QCString bak(afm);

            afm=user;
            afm+=bak;
        }
    }

    return CMisc::changeExt(afm, "afm");
}

QString CStarOfficeConfig::statusToStr(EStatus st)
{
    switch(st)
    {
        case SUCCESS: 
            return i18n("Success");
        case COULD_NOT_OPEN_X11_DIR:
            return i18n("Could not open X11 folder");
        case COULD_NOT_OPEN_PSSTD_FONTS:
            return i18n("Could not open psstd.fonts");
        case COULD_NOT_OPEN_PPD_FILE:
            return i18n("Could not open printer file");
        case COULD_NOT_OPEN_FONTS_DOT_SCALE:
            return i18n("Could not open X11 fonts.scale");
        case COULD_NOT_MODIFY_XPRINTER_DOT_PROLOG:
            return i18n("Could not modify xprinter.prolog");
        case COULD_NOT_LINK_XPRINTER_DOT_PROLOG:
            return i18n("Could not link xprinter.prolog");
        case COULD_NOT_RESTORE_XPRINTER_DOT_PROLOG:
            return i18n("Could not restore xprinter.prolog");
        default:
            return i18n("Unknown");
    }
}
#include "StarOfficeConfig.moc"
