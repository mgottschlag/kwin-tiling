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
    KAboutData *about = new KAboutData("kcm_pkk_authorization", "polkit-kde-authorization", ki18n("Polkit KDE Authorizations"), "0.1");
    setAboutData(about);
    setButtons(NoAdditionalButton);

    m_grid = new QGridLayout(this);
    view = new PolkitKde::PkKAuthorization;
    m_grid->addWidget(view);
}
