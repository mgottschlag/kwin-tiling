/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "KcmPkKAuthorization.h"

#include <KGenericFactory>
#include <KAboutData>

K_PLUGIN_FACTORY(PkKAuthorizationFactory, registerPlugin<KcmPkKAuthorization>();)
K_EXPORT_PLUGIN(PkKAuthorizationFactory("kcm_pkk_authorization"))

KcmPkKAuthorization::KcmPkKAuthorization(QWidget *parent, const QVariantList &args)
        : KCModule(PkKAuthorizationFactory::componentData(), parent, args)
{
    KGlobal::locale()->insertCatalog( "polkit-kde-authorization");
    // NOTE If you update aboutData here please do it at main.cpp too, thanks :D
    KAboutData *aboutData;
    aboutData = new KAboutData("polkit-kde-authorization",
                               "polkit-kde-authorization",
                               ki18n("PolicyKit KDE Authorization"),
                               "0.1",
                               ki18n("KDE interface for managing PolicyKit Authorizations"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2009 Daniel Nicoletti"));
    aboutData->addAuthor(ki18n("Daniel Nicoletti"), ki18n("Author"), "dantti85-pk@yahoo.com.br");
    aboutData->addAuthor(ki18n("Dario Freddi"), ki18n("Developer"), "drf54321@gmail.com", "http://drfav.wordpress.com");
    aboutData->addAuthor(ki18n("Alessandro Diaferia"), ki18n("Developer"), "alediaferia@gmail.com");
    aboutData->addAuthor(ki18n("Lukas Appelhans"), ki18n("Developer"), "l.appelhans@gmx.de", "http://boom1992.wordpress.com");
    aboutData->addAuthor(ki18n("Trever Fischer"), ki18n("Developer"), "wm161@wm161.net");

    setAboutData(aboutData);
    setButtons(NoAdditionalButton);

    m_grid = new QGridLayout(this);
    view = new PolkitKde::PkKAuthorization;
    m_grid->addWidget(view);
}
