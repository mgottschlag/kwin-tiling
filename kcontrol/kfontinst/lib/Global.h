#ifndef __GLOBAL_H__
#define __GLOBAL_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CGlobal
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/03/2003
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
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include "Misc.h"

class CFontEngine;
class CKfiConfig;
class CEncodings;
class CXConfig;
class KXftConfig;

#define KIO_FONTS_PROTOCOL "fonts"
#define KIO_FONTS_USER     I18N_NOOP("Personal")
#define KIO_FONTS_SYS      I18N_NOOP("System")

class CGlobal
{
    public:

    static void create(bool checkDirs=true, bool checkX=false);
    static void destroy();

    static CFontEngine & fe();
    static CKfiConfig &  cfg();
    static CEncodings &  enc();
    static CXConfig &    sysXcfg();
    static CXConfig &    userXcfg();
    static CXConfig &    xcfg()     { return CMisc::root() ? sysXcfg() : userXcfg(); }
    static KXftConfig &  userXft();
    static KXftConfig &  sysXft();
    static KXftConfig &  xft()      { return CMisc::root() ? sysXft() : userXft(); }

    private:

    static CFontEngine *theirFontEngine;
    static CKfiConfig  *theirConfig;
    static CEncodings  *theirEncodings;
    static CXConfig    *theirSysXcfg;
    static CXConfig    *theirUserXcfg;
    static KXftConfig  *theirSysXft;
    static KXftConfig  *theirUserXft;
};

#endif
