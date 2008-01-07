/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007 Harry Bock <hbock@providence.edu>
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
#include "randrscreen.h"
#include "randrmode.h"
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
	connect(activeCheck, SIGNAL(stateChanged(int)),
	        this, SIGNAL(optionChanged()));
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)), 
			this, SLOT(updateRateList(int)));
	//connect(m_output, SIGNAL(outputChanged(RROutput, int)), this, SLOT(load()));
	connect(m_output, SIGNAL(outputChanged(RROutput, int)),
	        this,     SLOT(outputChanged(RROutput, int)));
		   
	load();
}

OutputConfig::~OutputConfig()
{
}

/*
void OutputConfig::activeStateChanged(int)
{
	emit optionChanged(true);
}

	enum Changes 
	{
		ChangeCrtc       =  1,
		ChangeOutputs    =  2,
		ChangeMode       =  4,
		ChangeRotation   =  8,
		ChangeConnection = 16,
		ChangeRect       = 32,
		ChangeRate       = 64
	};
*/
void OutputConfig::outputChanged(RROutput output, int changes)
{
	Q_ASSERT(m_output->id() == output);
	kDebug() << "Output " << m_output->name() << " changed. (mask = " << changes << ")";
	
	if(changes & RandR::ChangeCrtc) {
		kDebug() << "Output CRTC changed";
		activeCheck->setChecked(m_output->isActive());
		
		updateSizeList();
		updateRateList();
	}
	
	if(changes & RandR::ChangeRect) {
		QRect r = m_output->rect();
		kDebug() << "Output rect changed: " << r;
		//m_item->setRect(0, 0, r.width(), r.height());
		m_item->setRect(r);
		//m_item->setPos
	}
	
	if(changes & RandR::ChangeRotation) {
		kDebug() << "Output rotation changed";
		updateRotationList();
	}
	
	if(changes & RandR::ChangeConnection) {
		kDebug() << "Output connection status changed";
		setEnabled(m_output->isConnected());
		activeCheck->setChecked(m_output->isActive());
	}
	
	if(changes & RandR::ChangeRate) {
		kDebug() << "Output rate changed";
		updateRateList();
	}
	
	if(changes & RandR::ChangeMode) {
		kDebug() << "Output mode changed.";
		// This NEEDS to be fixed..
		QSize modeSize = m_output->screen()->mode(m_output->mode()).size();
		
		updateRateList(sizeCombo->findData(modeSize));
	}
}

QString OutputConfig::positionName(RandROutput::Relation position)
{
	switch(position) {
	case RandROutput::LeftOf:  return i18n("Left of");
	case RandROutput::RightOf: return i18n("Right of");
	case RandROutput::Over:    return i18n("Above");
	case RandROutput::Under:   return i18n("Below");
	case RandROutput::SameAs:  return i18n("Clone of");
	}
	
	return i18n("No relative position");
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
	if (!m_output->isConnected())
		return;

	/* Mode size configuration */
	updateSizeList();
	
	/* Output Rotation Configuration */
	updateRotationList();
	updatePositionList();
	
	// update the item
	m_item->setRect( 0, 0, m_output->rect().width(), m_output->rect().height());
	kDebug() << "      --> setting graphic pos " << m_output->rect().topLeft();
	m_item->setPos( m_output->rect().topLeft() );

	emit updateView();
}

void OutputConfig::updatePositionList(void)
{
	RandROutput::Relation rel;
	m_output->relation(&rel);
	for(int i = 0; i < 5; i++)
		positionCombo->addItem(OutputConfig::positionName((RandROutput::Relation)i), i);
	
	int index = positionCombo->findData((int)rel);
	if(index != -1)
		positionCombo->setCurrentIndex(index);

	/* Relative Output Name Configuration
	 */
	OutputMap outputs = m_output->screen()->outputs();
	foreach(RandROutput *output, outputs)
		positionOutputCombo->addItem(QIcon(output->icon()), output->name(), (int)output->id());

	if(m_output->relation(0) != m_output) {
		index = positionOutputCombo->findData((int)m_output->relation(0)->id());
		if(index != -1)
			positionOutputCombo->setCurrentIndex(index);
	} else if(m_output->screen()->activeCount() < 2) {
		positionLabel->setVisible(false);
		positionCombo->setVisible(false);
		positionOutputCombo->setVisible(false);
	}
}

void OutputConfig::updateRotationList(void)
{
	int rotations = m_output->rotations();
	for(int i =0; i < 6; ++i) {
		int rot = (1 << i);
		if (rot & rotations) {
			orientationCombo->addItem(QIcon(RandR::rotationIcon(rot, RandR::Rotate0)), 
						  RandR::rotationName(rot), rot);
		}
	}
	
	int index = orientationCombo->findData(m_output->rotation());
	if (index != -1)
		orientationCombo->setCurrentIndex( index );
}

void OutputConfig::updateSizeList(void)
{	
	SizeList sizes = m_output->sizes();
	foreach (QSize s, sizes)	{
		sizeCombo->addItem( QString("%1x%2").arg(s.width()).arg(s.height()), s );
	}
	
	int index = sizeCombo->findData( m_output->rect().size() );
	if (index != -1)
		sizeCombo->setCurrentIndex( index );

	index = refreshCombo->findData(m_output->refreshRate());
	if (index != -1)
		refreshCombo->setCurrentIndex(index);
}

void OutputConfig::updateRateList(int resolutionIndex)
{
	QSize resolution = sizeCombo->itemData(resolutionIndex).toSize();
	if(!resolution.isValid()) {
		kDebug() << "Error, invalid QSize passed to updateRateList!";
		return;
	}
	
	ModeList modeList = m_output->modes();
	
	refreshCombo->clear();
	foreach(RRMode m, modeList) {
		RandRMode outMode = m_output->screen()->mode(m);
		if(outMode.isValid() && outMode.size() == resolution) {
			float rate = outMode.refreshRate();
			refreshCombo->addItem(i18n("%1 Hz", QString::number(rate, 'f', 1)), rate);
		}
	}
}

void OutputConfig::updateRateList()
{
	if (sizeCombo->currentIndex() == -1)
		return;

	// update the refresh rate list to reflect the currently selected
	// resolution
	updateRateList(sizeCombo->currentIndex());
}

#include "outputconfig.moc"
