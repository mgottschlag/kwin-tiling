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

#include "Global.h"
#include "FontEngine.h"
#include "Config.h"
#include "Encodings.h"
#include "XConfig.h"
#include "kxftconfig.h"   // CPD: This should really be a symlik!
#include <stdio.h>

CFontEngine * CGlobal::theirFontEngine=NULL;
CConfig     * CGlobal::theirConfig=NULL;
CEncodings  * CGlobal::theirEncodings=NULL;
CXConfig    * CGlobal::theirSysXcfg=NULL;
CXConfig    * CGlobal::theirUserXcfg=NULL;
KXftConfig  * CGlobal::theirSysXft=NULL;
KXftConfig  * CGlobal::theirUserXft=NULL;

void CGlobal::create(bool checkDirs, bool checkX)
{
    theirConfig=new CConfig(true, checkDirs, checkX);
}

void CGlobal::destroy()
{
    if(theirFontEngine)
        delete theirFontEngine;
    theirFontEngine=NULL;

    if(theirConfig)
        delete theirConfig;
    theirConfig=NULL;

    if(theirEncodings)
        delete theirEncodings;
    theirEncodings=NULL;

    if(theirSysXcfg)
        delete theirSysXcfg;
    theirSysXcfg=NULL;

    if(!CMisc::root() && theirUserXcfg)
        delete theirUserXcfg;
    theirUserXcfg=NULL;

    if(theirSysXft)
        delete theirSysXft;
    theirSysXft=NULL;

    if(!CMisc::root() && theirUserXft)
        delete theirUserXft;
    theirUserXft=NULL;
}

CFontEngine & CGlobal::fe()
{
    if(NULL==theirFontEngine)
        theirFontEngine=new CFontEngine;

    return *theirFontEngine;
}

CConfig & CGlobal::cfg()
{
    if(NULL==theirConfig)
        theirConfig=new CConfig;
 
    return *theirConfig;
}

CEncodings & CGlobal::enc()
{
    if(NULL==theirEncodings)
        theirEncodings=new CEncodings;
 
    return *theirEncodings;
}

CXConfig & CGlobal::sysXcfg()
{
    if(NULL==theirSysXcfg)
        theirSysXcfg=new CXConfig(cfg().getSysXfs() ? CXConfig::XFS : CXConfig::XF86, cfg().getSysXConfigFile());
    return *theirSysXcfg;
}

CXConfig & CGlobal::userXcfg()
{
    if(NULL==theirUserXcfg)
        if(CMisc::root())
            theirUserXcfg=&sysXcfg();
        else
            theirUserXcfg=new CXConfig(CXConfig::KFI, cfg().getUserXConfigFile());
    return *theirUserXcfg;
}

KXftConfig & CGlobal::userXft()
{
    if(NULL==theirUserXft)
        if(CMisc::root())
            theirUserXft=&sysXft();
        else
            theirUserXft=new KXftConfig(KXftConfig::Dirs|KXftConfig::SymbolFamilies, false);
    return *theirUserXft;
}

KXftConfig & CGlobal::sysXft()
{
    if(NULL==theirSysXft)
        theirSysXft=new KXftConfig(KXftConfig::Dirs|KXftConfig::SymbolFamilies, true);
    return *theirSysXft;
}
