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

class CCompressedFile
{
    public:

    CCompressedFile(const char *fname);
    virtual ~CCompressedFile() { close(); }

    operator bool()            { return itsGzip ? NULL!=itsGzFile : NULL!=itsFile; }

    void   close();
    int    read(void *data, unsigned int len);
    int    getChar();
    char * getString(char *data, unsigned int len);
    int    seek(int offset, int whence);
    bool   eof()               { return itsGzip ? gzeof(itsGzFile) : feof(itsFile); }

    private:

    // Don't really want copy contructor or operator=, therefore defined as private
    CCompressedFile & operator=(const CCompressedFile&);
    CCompressedFile(const CCompressedFile&);

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
