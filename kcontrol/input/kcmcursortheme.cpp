/*
 *  Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcmcursortheme.h"

#include <KDE/KAboutData>
#include <KDE/KPluginFactory>

K_PLUGIN_FACTORY(CursorThemeConfigFactory,
    registerPlugin<CursorThemeConfig>();
)
K_EXPORT_PLUGIN(CursorThemeConfigFactory("kcm_cursortheme", "kcminput"))


CursorThemeConfig::CursorThemeConfig(QWidget *parent, const QVariantList &args)
    : KCModule(CursorThemeConfigFactory::componentData(), parent, args)
{
    QLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);

    themepage = new ThemePage(this);
    connect(themepage, SIGNAL(changed(bool)), SLOT(changed()));
    layout->addWidget(themepage);

    KAboutData* aboutData = new KAboutData("kcm_cursortheme", 0, ki18n("Cursor Theme"),
        0, KLocalizedString(), KAboutData::License_GPL, ki18n("(c) 2003-2007 Fredrik Höglund"));
    aboutData->addAuthor(ki18n("Fredrik Höglund"));
    setAboutData(aboutData);
}

CursorThemeConfig::~CursorThemeConfig()
{
    /* */
}

void CursorThemeConfig::load()
{
    themepage->load();
    emit changed(false);
}

void CursorThemeConfig::save()
{
    themepage->save();
    emit changed(false);
}

void CursorThemeConfig::defaults()
{
    themepage->defaults();
    changed();
}

#include "kcmcursortheme.moc"
