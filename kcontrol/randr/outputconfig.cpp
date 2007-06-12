/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include "outputconfig.h"
#include "randroutput.h"

OutputConfig::OutputConfig(QWidget *parent, RandROutput *output)
: QWidget(parent)
{
	m_output = output;
	Q_ASSERT(output);
	setupUi(this);

	connect(activeCheck, SIGNAL(checked(bool)), sizeCombo, SLOT(setEnabled(bool)));
	connect(activeCheck, SIGNAL(checked(bool)), rotationGroup, SLOT(setEnabled(bool)));

	rotate0->setText(RandR::rotationName(RandR::Rotate0));
	rotate0->setIcon(RandR::rotationIcon(RandR::Rotate0, RandR::Rotate0));
	rotate90->setText(RandR::rotationName(RandR::Rotate90));
	rotate90->setIcon(RandR::rotationIcon(RandR::Rotate90, RandR::Rotate0));
	rotate180->setText(RandR::rotationName(RandR::Rotate180));
	rotate180->setIcon(RandR::rotationIcon(RandR::Rotate180, RandR::Rotate0));
	rotate270->setText(RandR::rotationName(RandR::Rotate270));
	rotate270->setIcon(RandR::rotationIcon(RandR::Rotate270, RandR::Rotate0));
}

OutputConfig::~OutputConfig()
{
}

#include "outputconfig.moc"
