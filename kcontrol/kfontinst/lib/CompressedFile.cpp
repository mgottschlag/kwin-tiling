//////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CCompressedFile
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "CompressedFile.h"
#ifdef KFI_USE_KFILTERDEV
#include <kfilterdev.h>
#endif
#include <kprocess.h>
#include <qfile.h>

CCompressedFile::CCompressedFile(const QString &fname) 
               : itsType(NORM),
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

static CCompressedFile::EType getType(const QString &fname)
{
    // Check for .gz...
    if(fname.endsWith(".gz"))
        return CCompressedFile::GZIP;

    // Check for .Z
    if(fname.endsWith(".Z"))
        return CCompressedFile::Z;

    // Else assume its a normal file...
        return CCompressedFile::NORM;
}

void CCompressedFile::open(const QString &fname)
{
    itsType=getType(fname);
    itsFName=fname;
    itsPos=0;

    switch(itsType)
    {
        case GZIP:
#ifdef KFI_USE_KFILTERDEV
            itsDev=KFilterDev::deviceForFile(fname, "application/x-gzip");
            if(itsDev && !itsDev->open(IO_ReadOnly))
                close();
#else
            itsGzFile=gzopen(QFile::encodeName(fname), "r");
#endif
            break;
        case Z:
        {
            QString cmd = "uncompress -c " + KProcess::quote(fname);
            itsFile=popen(QFile::encodeName(cmd), "r");
            break;
        }
        case NORM:
            itsFile=fopen(QFile::encodeName(fname), "r");
    }
}

void CCompressedFile::close()
{
    if(*this)
        switch(itsType)
        {
            case GZIP:
#ifdef KFI_USE_KFILTERDEV
                if(itsDev)
                    delete itsDev;
                itsDev=NULL;
#else
                gzclose(itsGzFile);
                itsGzFile=NULL;
#endif
                break;
            case Z:
                while(!eof())
                    getChar();
                pclose(itsFile);
                itsFile=NULL;
                break;
            case NORM:
                fclose(itsFile);
                itsFile=NULL;
        }
}

int CCompressedFile::read(void *data, unsigned int len)
{
    int r=0;

    if(GZIP==itsType)
#ifdef KFI_USE_KFILTERDEV
    {
        if(itsDev)
            r=itsDev->readBlock((char *)data, len);
    }
#else
        r=gzread(itsGzFile, data, len);
#endif
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
#ifdef KFI_USE_KFILTERDEV
    {
        if(itsDev)
            c=itsDev->getch();
    }
#else
        c=gzgetc(itsGzFile);
#endif
    else
        c=fgetc(itsFile);

    if(EOF!=c)
        itsPos++;
    return c;
}

#ifndef KFI_USE_KFILTERDEV
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
#endif

char * CCompressedFile::getString(char *data, unsigned int len)
{
    char *s=NULL;

    if(GZIP==itsType)
#ifdef KFI_USE_KFILTERDEV
    {
        if(itsDev)
            s=itsDev->readLine(data, len)!=-1 ? data : NULL;
    }
#else
        s=kfi_gzgets(itsGzFile, data, len);
#endif
    else
        s=fgets(data, len, itsFile);

    if(NULL!=s)
        itsPos+=strlen(s);

    return s;
}

int CCompressedFile::seek(int offset, int whence)
{
    if(NORM==itsType)
        return fseek(itsFile, offset, whence);
    else
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
