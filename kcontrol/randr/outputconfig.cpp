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
#include "outputgraphicsitem.h"
#include "randroutput.h"
#include <kdebug.h>

OutputConfig::OutputConfig(QWidget *parent, RandROutput *output, OutputGraphicsItem *item)
: QWidget(parent)
{
	m_output = output;
	Q_ASSERT(output);

	m_item = item;
	Q_ASSERT(item);

	setupUi(this);

	// connect signals
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)), 
			this, SLOT(loadRefreshRates()));
	connect(m_output, SIGNAL(outputChanged(RROutput, int)), this, SLOT(load()));
	
	load();
}

OutputConfig::~OutputConfig()
{
}

void OutputConfig::load()
{
	int index;

	kDebug() << "Output Load......";
	setEnabled( m_output->isConnected() );
	activeCheck->setChecked(m_output->isActive());

	sizeCombo->clear();
	orientationCombo->clear();

	m_item->setVisible(m_output->isActive());	
	if (!m_output->isActive())
		return;

	// load sizes
	SizeList sizes = m_output->sizes();
	foreach (QSize s, sizes)
	{
		sizeCombo->addItem( QString("%1x%2").arg(s.width()).arg(s.height()), s );
	}
		
	index = sizeCombo->findData( m_output->rect().size() );
	if (index != -1)
		sizeCombo->setCurrentIndex( index );

	index = refreshCombo->findData(m_output->refreshRate());
	if (index != -1)
		refreshCombo->setCurrentIndex(index);

	int rotations = m_output->rotations();
	for(int i =0; i < 6; ++i)
	{
		if ((1 << i) & rotations)
		{
			orientationCombo->addItem(QIcon(RandR::rotationIcon(1 << i, RandR::Rotate0)), 
						  RandR::rotationName(1 << i), (1 << i));
		}
	}
	index = orientationCombo->findData(m_output->rotation());
	if (index != -1)
		orientationCombo->setCurrentIndex( index );

	// update the item
	m_item->setRect( 0, 0, m_output->rect().width(), m_output->rect().height());
	kDebug() << "      --> setting pos " << m_output->rect().topLeft();
	m_item->setPos( m_output->rect().topLeft() );

	emit updateView();
}

void OutputConfig::loadRefreshRates()
{
	refreshCombo->clear();
	if (sizeCombo->currentIndex() == -1)
		return;

	RateList rates = m_output->refreshRates();
	foreach(float rate, rates)
	{
		refreshCombo->addItem(i18n("%1 Hz", QString::number(rate, 'f', 1)), rate);
	}
	// select the first item
	refreshCombo->setCurrentIndex( 0 );
}

#include "outputconfig.moc"
