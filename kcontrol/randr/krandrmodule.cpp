/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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

#include "krandrmodule.h"
#include "legacyrandrconfig.h"
#include <QTextStream>
#include "legacyrandrscreen.h"
#include "randrdisplay.h"
#include "randrconfig.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KDebug>
#include <KApplication>
#include <config-randr.h>

#include "randr.h"

// DLL Interface for kcontrol
K_PLUGIN_FACTORY(KSSFactory, registerPlugin<KRandRModule>();)
K_EXPORT_PLUGIN(KSSFactory("krandr"))

KRandRModule::KRandRModule(QWidget *parent, const QVariantList&)
    : KCModule(KSSFactory::componentData(), parent)
{
	m_display = new RandRDisplay();
	if (!m_display->isValid())
	{
		QVBoxLayout *topLayout = new QVBoxLayout(this);
		QLabel *label =
		    new QLabel(i18n("Your X server does not support resizing and "
		                    "rotating the display. Please update to version 4.3 "
						"or greater. You need the X Resize, Rotate, and Reflect "
						"extension (RANDR) version 1.1 or greater to use this "
						"feature."), this);
						
		label->setWordWrap(true);
		topLayout->addWidget(label);
		kWarning() << "Error: " << m_display->errorCode() ;
		return;
	}

	QVBoxLayout* topLayout = new QVBoxLayout(this);
	topLayout->setMargin(0);
	topLayout->setSpacing(KDialog::spacingHint());

#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
	{
		m_config = new RandRConfig(this, m_display);
		connect(m_config, SIGNAL(changed(bool)), SIGNAL(changed(bool)));
		topLayout->addWidget(m_config);
	}
	else
#endif
	{
		m_legacyConfig = new LegacyRandRConfig(this, m_display);
		connect(m_legacyConfig, SIGNAL(changed(bool)), SIGNAL(changed(bool)));
		topLayout->addWidget(m_legacyConfig);
	}

	//topLayout->addStretch(1);

	setButtons(KCModule::Apply);

	kapp->installX11EventFilter( this );
}

KRandRModule::~KRandRModule(void)
{
	delete m_display;
}

void KRandRModule::defaults()
{
        if (!m_display->isValid()) {
                return;
        }
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		m_config->defaults();
	else
#endif
		m_legacyConfig->defaults();
}

void KRandRModule::load()
{
	kDebug() << "Loading KRandRModule...";
	
        if (!m_display->isValid()) {
                return;
        }
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		m_config->load();
	else
#endif
		m_legacyConfig->load();

	emit changed(false);
}

void KRandRModule::save()
{
        if (!m_display->isValid()) {
                return;
        }
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		m_config->save();
	else
#endif
		m_legacyConfig->save();

}

void KRandRModule::apply()
{
        if (!m_display->isValid()) {
                return;
        }
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		m_config->apply();
	else
#endif
		m_legacyConfig->apply();
}

bool KRandRModule::x11Event(XEvent* e)
{
	if (m_display->canHandle(e)) {
		m_display->handleEvent(e);
	}
	return QWidget::x11Event(e);
}


#include "krandrmodule.moc"
