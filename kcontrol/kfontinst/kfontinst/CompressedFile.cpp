//////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CCompressedFile
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "CompressedFile.h"
#include <kfilterdev.h>
#include <kprocess.h>

#include <qfile.h>

CCompressedFile::CCompressedFile(const QString &fname) 
 : itsType(NORM), itsFName(fname), itsFile(NULL) 
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
            itsDev=KFilterDev::deviceForFile(fname, "application/x-gzip");
            if(itsDev && !itsDev->open(IO_ReadOnly))
                close();
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
                delete itsDev;
                itsDev=NULL;
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
    {
        if(itsDev)
            r=itsDev->readBlock((char *)data, len);
    }
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
    {
        if(itsDev)
            c=itsDev->getch();
    }
    else
        c=fgetc(itsFile);

    if(EOF!=c)
        itsPos++;
    return c;
}

char * CCompressedFile::getString(char *data, unsigned int len)
{
    char *s=NULL;

    if(GZIP==itsType)
    {
        if(itsDev)
            s=itsDev->readLine(data, len)!=-1 ? data : NULL;
    }
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
