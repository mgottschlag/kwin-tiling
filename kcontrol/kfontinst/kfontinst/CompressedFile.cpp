//////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CCompressedFile
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 29/11/2001
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "CompressedFile.h"
#include <kprocess.h>
#include <qfile.h>

namespace KFI
{

CCompressedFile::CCompressedFile(const QString &fname) 
               : itsType(GZIP),
                 itsFName(fname),
                 itsFile(NULL) 
{ 
    if(!fname.isEmpty())
        open(fname); 
}

CCompressedFile::~CCompressedFile() 
{ 
    close();
}

void CCompressedFile::open(const QString &fname)
{
    itsType=fname.endsWith(".Z") ? CCompressedFile::Z : CCompressedFile::GZIP;
    itsFName=fname;
    itsPos=0;

    switch(itsType)
    {
        case GZIP:
            itsGzFile=gzopen(QFile::encodeName(fname), "r");
            break;
        case Z:
        {
            QString cmd = "uncompress -c " + KProcess::quote(fname);
            itsFile=popen(QFile::encodeName(cmd), "r");
            break;
        }
    }
}

void CCompressedFile::close()
{
    if(*this)
        switch(itsType)
        {
            case GZIP:
                gzclose(itsGzFile);
                itsGzFile=NULL;
                break;
            case Z:
                while(!eof())
                    getChar();
                pclose(itsFile);
                itsFile=NULL;
                break;
        }
}

int CCompressedFile::read(void *data, unsigned int len)
{
    int r=0;

    if(GZIP==itsType)
        r=gzread(itsGzFile, data, len);
    else
        r=fread(data, 1, len, itsFile);

    if(r>0)
        itsPos+=r;
    return r;
}

int CCompressedFile::getChar()
{
    int c=EOF;

    if(GZIP==itsType)
        c=gzgetc(itsGzFile);
    else
        c=fgetc(itsFile);

    if(EOF!=c)
        itsPos++;
    return c;
}

// Copied from zlib 1.2.1 source as some installtion seem not to have gzgets()
char * kfi_gzgets(gzFile file, char *buf, int len)
{
    char *b=buf;

    if (Z_NULL==buf || len <= 0)
        return Z_NULL;

    while(--len > 0 && gzread(file, buf, 1) == 1 && *buf++ != '\n')
        ;
    *buf = '\0';

    return b == buf && len > 0 ? Z_NULL : b;
}

char * CCompressedFile::getString(char *data, unsigned int len)
{
    char *s=NULL;

    if(GZIP==itsType)
        s=kfi_gzgets(itsGzFile, data, len);
    else
        s=fgets(data, len, itsFile);

    if(NULL!=s)
        itsPos+=strlen(s);

    return s;
}

int CCompressedFile::seek(int offset, int whence)
{
    int c;

    switch(whence)
    {
        case SEEK_CUR:
            break;
        case SEEK_SET:
            if(offset<itsPos)
            {
                close();
                open(itsFName);
            }
            offset-=itsPos;
            break;
        default:
            offset=-1;
    }

    for(c=0; c<offset && -1!=getChar(); c++)
        ;
    return c==offset ? 0 : -1;
}

}
