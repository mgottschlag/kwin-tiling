////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfiGlobal
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
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

#include "KfiGlobal.h"
#include "FontEngine.h"
#include "Encodings.h"
#include "Config.h"
#include "ErrorDialog.h"
#include "Ttf.h"
#include "XConfig.h"
#include "XftConfig.h"
#include <qwidget.h>
#include <stdio.h>

CFontEngine  * CKfiGlobal::theirFontEngine=NULL;
CConfig      * CKfiGlobal::theirConfig=NULL;
CEncodings   * CKfiGlobal::theirEncodings=NULL;
CErrorDialog * CKfiGlobal::theirErrorDialog=NULL;
CTtf         * CKfiGlobal::theirTtf=NULL;
CXConfig     * CKfiGlobal::theirXCfg=NULL;
#ifdef HAVE_XFT
CXftConfig   * CKfiGlobal::theirXft=NULL;
#endif

void CKfiGlobal::create(QWidget *parent)
{
    fe();
    cfg();
    enc();
    ttf();
    xcfg();
#ifdef HAVE_XFT
    xft();
#endif
    if(NULL==theirErrorDialog)
        theirErrorDialog=new CErrorDialog(parent);
}

void CKfiGlobal::destroy()
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

    if(theirTtf)
        delete theirTtf;
    theirTtf=NULL;

    if(theirXCfg)
        delete theirXCfg;
    theirXCfg=NULL;
#ifdef HAVE_XFT
    if(theirXft)
        delete theirXft;
    theirXft=NULL;
#endif

    if(theirErrorDialog)
        delete theirErrorDialog;
    theirErrorDialog=NULL;
}

CFontEngine & CKfiGlobal::fe()
{
    if(NULL==theirFontEngine)
        theirFontEngine=new CFontEngine;

    return *theirFontEngine;
}

CConfig & CKfiGlobal::cfg()
{
    if(NULL==theirConfig)
        theirConfig=new CConfig;
 
    return *theirConfig;
}

CEncodings & CKfiGlobal::enc()
{
    if(NULL==theirEncodings)
        theirEncodings=new CEncodings(cfg().getEncodingsDir());
 
    return *theirEncodings;
}

CErrorDialog & CKfiGlobal::errorDialog()
{
    if(NULL==theirErrorDialog)
        theirErrorDialog=new CErrorDialog(NULL);

    return *theirErrorDialog;
}

CTtf & CKfiGlobal::ttf()
{
    if(NULL==theirTtf)
        theirTtf=new CTtf;

    return *theirTtf;
}

CXConfig & CKfiGlobal::xcfg()
{
    if(NULL==theirXCfg)
        theirXCfg=new CXConfig;

    return *theirXCfg;
}

#ifdef HAVE_XFT
CXftConfig & CKfiGlobal::xft()
{
    if(NULL==theirXft)
    {
        theirXft=new CXftConfig;
        theirXft->read(cfg().getXftConfigFile());
    }
 
    return *theirXft;
}
#endif
