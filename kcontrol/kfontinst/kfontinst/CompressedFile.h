#ifndef __COMPRESSED_FILE_H__
#define __COMPRESSED_FILE_H__

//////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CCompressedFile
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 28/11/2001
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

#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

class CCompressedFile
{
    public:

    CCompressedFile(const char *fname)
    {
        unsigned int len=fname ? strlen(fname) : 0;

        itsGzip=len ? !(fname[len-2]=='.' && toupper(fname[len-1])=='Z') : true;

        if(itsGzip)
            itsGzFile=gzopen(fname, "r");
        else
        {
            const unsigned int constCmdSize=1024;

            if(len+20<constCmdSize)
            {
                char cmd[constCmdSize];

                sprintf(cmd, "uncompress -c \"%s\"", fname);
                itsFile=popen(cmd, "r");
                itsPos=0;
            }
            else
                itsFile=NULL;
        }
    }

    ~CCompressedFile()
    {
        close();
    }

    void close()
    {
        if(*this)
            if(itsGzip)
            {
                gzclose(itsGzFile);
                itsGzFile=NULL;
            }
            else
            {
                pclose(itsFile);
                itsFile=NULL;
            }
    }

    operator bool() { return itsGzip ? NULL!=itsGzFile : NULL!=itsFile; }

    int read(void *data, unsigned int len)
    {
        if(itsGzip)
            return gzread(itsGzFile, data, len);
        else
        {
            int r=fread(data, 1, len, itsFile);
            if(r>0)
                itsPos+=r;
            return r;
        }
    }

    int getChar()
    {
        if(itsGzip)
            return gzgetc(itsGzFile);
        else
        {
            int c=fgetc(itsFile);
            if(EOF!=c)
                itsPos++;
            return c;
        }
    }

    char * getString(char *data, unsigned int len)
    {
        if(itsGzip)
            return gzgets(itsGzFile, data, len);
        else
        {
            char *s=fgets(data, len, itsFile);
            if(NULL!=s)
                itsPos+=strlen(s);
            return s;
        }
    }

    int seek(int offset, int whence)
    {
        if(itsGzip)
            return gzseek(itsGzFile, offset, whence);
        else
        {
            switch(whence)
            {
                case SEEK_CUR:
                    break;
                case SEEK_SET:
                    offset-=itsPos;
                    break;
                default:
                    offset=-1;
            }

            char ch;
            int  c;

            for(c=0; c<offset && read(&ch, 1); c++)
                itsPos++;

            return c==offset ? offset : -1;
        }
    }

    bool eof()
    {
        if(itsGzip)
            return gzeof(itsGzFile);
        else
            return feof(itsFile);
    }

    private:

    bool itsGzip;
    int  itsPos;

    union
    {
        FILE   *itsFile;
        gzFile itsGzFile;
    };
};

#endif
