////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFontViewPartFactory
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/08/2002
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
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

#include "kfontviewpart_factory.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <assert.h>

#include "kfontviewpart.h"

extern "C"
{
    void* init_libkfontviewpart()
    {
	return new KFontViewPartFactory;
    }
};

KInstance * KFontViewPartFactory::s_instance=0L;
KAboutData * KFontViewPartFactory::s_about=0L;

KFontViewPartFactory::KFontViewPartFactory()
{
}

KFontViewPartFactory::~KFontViewPartFactory()
{
    delete s_about;
    s_about=0L;
    delete s_instance;
    s_instance=0L;
}

QObject * KFontViewPartFactory::createObject(QObject *parent, const char *name, const char *, const QStringList &)
{
    if(parent && !parent->inherits("QWidget"))
    {
        kdDebug() << "KFontViewPartFactory: parent does not inherit QWidget" << endl;
        return 0L;
    }

    return new KFontViewPart((QWidget*) parent, name);
}

KInstance* KFontViewPartFactory::instance()
{
    if(!s_instance)
    {
        s_about = new KAboutData( "fontviewpart", I18N_NOOP( "KFontViewPart" ), "0.1" );
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

#include "kfontviewpart_factory.moc"
