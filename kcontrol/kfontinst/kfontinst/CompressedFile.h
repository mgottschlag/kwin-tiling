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

#include <stdio.h>
#include <qiodevice.h>

class CCompressedFile
{
    public:

    enum EType
    {
        GZIP,
        Z,
        NORM
    };

    CCompressedFile(const char *fname) : itsType(NORM), itsFName(fname), itsFile(NULL) { if(NULL!=fname) open(fname); }
    virtual ~CCompressedFile()                                                         { close(); }

    operator bool()                                                                    { return GZIP==itsType ? NULL!=itsDev : NULL!=itsFile; }

    void   open(const char *fname);
    void   close();
    int    read(void *data, unsigned int len);
    int    getChar();
    char * getString(char *data, unsigned int len);
    int    seek(int offset, int whence);
    bool   eof()                                                                       { return GZIP==itsType ? itsDev && itsDev->atEnd() : feof(itsFile); }

    private:

    // Don't really want copy contructor or operator=, therefore defined as private
    CCompressedFile & operator=(const CCompressedFile&);
    CCompressedFile(const CCompressedFile&);

    private:

    EType       itsType;
    int         itsPos;
    const char  *itsFName;

    union
    {
        FILE      *itsFile;
        QIODevice *itsDev;
    };
};

#endif
