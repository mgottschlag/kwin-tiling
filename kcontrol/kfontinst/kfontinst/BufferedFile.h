#ifndef __BUFFERED_FILE__
#define __BUFFERED_FILE__

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

#include <fstream.h>
#include <qcstring.h>

class CBufferedFile
{
    public:

    CBufferedFile(const QCString &file, const QCString &guard, const char *insertPos=NULL, bool insertBefore=true, bool section=false,
                  bool guardFirst=false);
    ~CBufferedFile();

    void write(const QCString &str);
    void writeNoGuard(const QCString &str);

    static QCString createGuard(const QCString &normGuard, const QCString &path, bool small=true);

    void close();

    operator bool() { return itsFile; }

    private:

    char         *itsData;
    unsigned int itsSize,
                 itsOffset;
    ofstream     itsFile;
    QCString     itsGuard;
    bool         itsSection, // i.e. is the stuff to be removed marked on each line, or in  a section...
                 itsWrittenGuard;
};

#endif
