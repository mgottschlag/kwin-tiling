////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CBufferdFile
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/05/2001
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

#include "BufferedFile.h"
#include "Misc.h"
#include <string.h>
#include <stdlib.h>

using namespace std;

CBufferedFile::CBufferedFile(const QCString &file, const QCString &guard, const char *insertPos, bool insertBefore, bool section)
             : itsData(NULL),
               itsSize(0),
               itsOffset(0),
               itsGuard(guard),
               itsSection(section),
               itsWrittenGuard(false)
{
    ifstream in(file);

    if(in)
    {
        in.seekg(0, ios::end);
        itsSize=in.tellg();

        if(itsSize>0)
        {
            itsData=new char [itsSize];

            if(itsData)
            {
                const int constMaxLineLen=4096;

                char         buffer[constMaxLineLen];
                unsigned int offset=0;
                bool         error=false,
                             foundSection=false,
                             ignoreLine=false,
                             useNextLine=false;

                in.seekg(0, ios::beg);

                do
                {
                    in.getline(buffer, constMaxLineLen);

                    if(in.good())
                    {
                        buffer[constMaxLineLen-1]='\0';

                        unsigned int dataLen=strlen(buffer);

                        if(offset+dataLen>itsSize)
                        {
                            error=true;
                            break;
                        }

                        if(section)
                            if(ignoreLine && useNextLine)
                                ignoreLine=useNextLine=section=false;  // Also ignore section checking from now on...
                            else
                                if(!foundSection && strstr(buffer, guard)==buffer)
                                    foundSection=ignoreLine=true;        // Ignore the next few lines, until guard is seen again
                                else
                                    if(foundSection && strstr(buffer, guard)==buffer)
                                        useNextLine=true;   // Next time around do-while, use the lines

                        if(!ignoreLine && ('#'==buffer[0] || strstr(buffer, guard)==NULL))
                        {
                            memcpy(&itsData[offset], buffer, dataLen);
                            itsData[offset+dataLen]='\n';
                            if(0==itsOffset && NULL!=insertPos && strstr(buffer, insertPos)==buffer)
                                if(insertBefore)
                                    itsOffset=offset;
                                else
                                    itsOffset=offset+dataLen+1;

                            offset+=dataLen+1;
                        }
                    }
                }
                while(!in.eof());
                if(!error)
                    itsSize=offset;
            }
        }

        in.close();
    }

    itsFile.open(file);

    if(itsFile && itsOffset>0 && itsSize>0 && itsOffset<=itsSize)
        itsFile.write(itsData, itsOffset);
}

CBufferedFile::~CBufferedFile()
{
    close();
}

void CBufferedFile::write(const QCString &str)
{
    if(itsFile)
        if(itsSection)
        {
            if(!itsWrittenGuard)
            {
                itsFile << itsGuard << endl;
                itsWrittenGuard=true;
            }
            itsFile << str << endl;
        }
        else
            itsFile << str << itsGuard << endl;
}

void CBufferedFile::writeNoGuard(const QCString &str)
{
    if(itsFile)
        itsFile << str << endl;
}

void CBufferedFile::close()
{
    if(itsSection && itsWrittenGuard) // then write terminating guard...
    {
        itsFile << itsGuard << endl;
        itsWrittenGuard=false;
    }

    if(itsData)
    {
        if(itsData && itsOffset<itsSize && itsFile)
            itsFile.write(&itsData[itsOffset], itsSize-itsOffset);

        delete itsData;
        itsData=NULL;
    }

    if(itsFile)
        itsFile.close();
}

QCString CBufferedFile::createGuard(const QCString &normGuard, const QCString &path, bool small)
{
    QCString guard(normGuard);
 
    if(!CMisc::root())
    {
        char *user=getlogin();
 
        if(!user)
            user=getenv("LOGNAME");
        if(!user)
            user=getenv("USER");
 
        if(user)
        {
            guard+=user;
            if(!small)
                guard+=" ";
        }
    }

    if(small) 
    {
        guard+=CMisc::shortName(path);
        guard.replace(QRegExp("/"), "");
    }
    else
        guard+=path;
 
    return guard;
}
