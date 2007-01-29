/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FontViewPartFactory.h"
#include "FontViewPart.h"
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <assert.h>

extern "C"
{
    KDE_EXPORT void* init_libkfontviewpart()
    {
        KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
        return new KFI::CFontViewPartFactory;
    }
}

namespace KFI
{

KComponentData *CFontViewPartFactory::theirInstance=NULL;
KAboutData * CFontViewPartFactory::theirAbout=NULL;

CFontViewPartFactory::CFontViewPartFactory()
{
}

CFontViewPartFactory::~CFontViewPartFactory()
{
    delete theirAbout;
    theirAbout=0L;
    delete theirInstance;
    theirInstance=0L;
}

QObject * CFontViewPartFactory::createObject(QObject *parent, const char *, const QStringList &)
{
    if(parent && !parent->isWidgetType())
    {
        kDebug() << "CFontViewPartFactory: parent does not inherit QWidget" << endl;
        return 0L;
    }

    return new CFontViewPart((QWidget*) parent);
}

const KComponentData &CFontViewPartFactory::componentData()
{
    if(!theirInstance)
    {
        theirAbout = new KAboutData("fontviewpart", I18N_NOOP("CFontViewPart"), "0.1");
        theirInstance = new KComponentData(theirAbout);
    }
    return *theirInstance;
}

}

#include "FontViewPartFactory.moc"
