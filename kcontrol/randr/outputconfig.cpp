/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
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
	connect(positionCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(positionComboChanged(int)));
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(updateRateList(int)));
	connect(m_output, SIGNAL(outputChanged(RROutput, int)),
	        this,     SLOT(outputChanged(RROutput, int)));
		  
	connect(sizeCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(refreshCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(orientationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(positionCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(positionOutputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	
	load();
}

OutputConfig::~OutputConfig()
{
}

RandROutput *OutputConfig::output(void) const
{
	return m_output;
}

QPoint OutputConfig::position(void) const
{
	int index = positionCombo->currentIndex();
	if((Relation)positionCombo->itemData(index).toInt() == Absolute)
		return QPoint(absolutePosX->text().toInt(), absolutePosY->text().toInt());
	
	return QPoint(0, 0);
}

QSize OutputConfig::resolution(void) const
{
	return sizeCombo->itemData(sizeCombo->currentIndex()).toSize();
}

float OutputConfig::refreshRate(void) const
{
	float rate = float(refreshCombo->itemData(refreshCombo->currentIndex()).toDouble());
	if(rate == 0.0f) {
		RateList rates = m_output->refreshRates(resolution());
		return rates.first();
	}
	return rate;
}

int OutputConfig::rotation(void) const
{
	return orientationCombo->itemData(orientationCombo->currentIndex()).toInt();
}

void OutputConfig::outputChanged(RROutput output, int changes)
{
	Q_ASSERT(m_output->id() == output);
	kDebug() << "Output " << m_output->name() << " changed. (mask = " << changes << ")";
	
	if(changes & RandR::ChangeOutputs) {
		kDebug() << "Outputs changed";
	}
	
	if(changes & RandR::ChangeCrtc) {
		kDebug() << "Output CRTC changed";
		
		updateSizeList();
		updateRateList();
		updateRotationList();
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
	}
	
	if(changes & RandR::ChangeRate) {
		kDebug() << "Output rate changed";
		updateRateList();
	}
	
	if(changes & RandR::ChangeMode) {
		kDebug() << "Output mode changed.";
		// This NEEDS to be fixed..
		//QSize modeSize = m_output->screen()->mode(m_output->mode()).size();
		QSize modeSize = m_output->mode().size();
		updateRateList(sizeCombo->findData(modeSize));
	}
}

QString OutputConfig::positionName(Relation position)
{
	switch(position) {
	case LeftOf:  return i18n("Left of");
	case RightOf: return i18n("Right of");
	case Over:    return i18nc("Output is placed above another one", "Above");
	case Under:   return i18nc("Output is placed below another one", "Below");
	case SameAs:  return i18n("Clone of");
	case Absolute:  return i18nc("Fixed, abitrary position", "Absolute");
	}
	
	return i18n("No relative position");
}

void OutputConfig::load()
{
	kDebug() << "Loading output configuration for " << m_output->name();
	setEnabled( m_output->isConnected() );

	sizeCombo->clear();
	orientationCombo->clear();

	m_item->setVisible(m_output->isActive());	
	if (!m_output->isConnected())
		return;

	/* Mode size configuration */
	updateSizeList();
	
	/* Output rotation and relative position */
	updateRotationList();
	updatePositionList();
	
	// update the item
	m_item->setRect( 0, 0, m_output->rect().width(), m_output->rect().height());
	kDebug() << "  Setting graphic rect pos: " << m_output->rect().topLeft();
	m_item->setPos( m_output->rect().topLeft() );

	emit updateView();
}

void OutputConfig::setConfigDirty(void)
{
	m_changed = true;
	emit optionChanged();
}

void OutputConfig::positionComboChanged(int item)
{
	Relation rel;
	rel = (Relation)positionCombo->itemData(item).toInt();
	
	bool isAbsolute = (rel == Absolute);
	
	positionOutputCombo->setVisible(!isAbsolute);
	absolutePosX->setVisible(isAbsolute);
	absolutePosY->setVisible(isAbsolute);
	
	if(isAbsolute) {
		int posX = m_output->rect().topLeft().x();
		int posY = m_output->rect().topLeft().y();
		
		absolutePosX->setText(QString::number(posX));
		absolutePosY->setText(QString::number(posY));
	}
}

void OutputConfig::updatePositionList(void)
{
	Relation rel = SameAs;
	// FIXME: get default value from KConfig
	for(int i = -1; i < 5; i++)
		positionCombo->addItem(OutputConfig::positionName((Relation)i), i);
	
	int index = positionCombo->findData((int)rel);
	if(index != -1)
		positionCombo->setCurrentIndex(index);

	/* Relative Output Name Configuration */
	OutputMap outputs = m_output->screen()->outputs();
	foreach(RandROutput *output, outputs)
		positionOutputCombo->addItem(QIcon(output->icon()), output->name(), (int)output->id());

	// FIXME: get this from Kconfig again
	/*if(m_output->relation(0) != m_output) {
		index = positionOutputCombo->findData((int)m_output->relation(0)->id());
		if(index != -1)
			positionOutputCombo->setCurrentIndex(index);
	} else*/ if(m_output->screen()->activeCount() < 2) {
		positionLabel->setEnabled(false);
		positionCombo->setEnabled(false);
		positionOutputCombo->setEnabled(false);
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
	sizeCombo->addItem( i18n("Disabled"), QSize(0, 0) );
	
	foreach (QSize s, sizes) {
		QString sizeDesc = QString("%1x%2").arg(s.width()).arg(s.height());		
		if(s == m_output->preferredMode().size())
			sizeDesc += i18nc("Automatic (native resolution)", " (Auto)");
		
		sizeCombo->addItem( sizeDesc, s );
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
	if((resolution == QSize(0, 0)) || !resolution.isValid()) {
		refreshCombo->setEnabled(false);
		return;
	}
	
	ModeList modeList = m_output->modes();
	
	refreshCombo->clear();
	refreshCombo->addItem(i18nc("Automatic configuration", "Auto"), 0.0f);
	refreshCombo->setEnabled(true);
	foreach(RRMode m, modeList) {
		RandRMode outMode = m_output->screen()->mode(m);
		if(outMode.isValid() && outMode.size() == resolution) {
			float rate = outMode.refreshRate();
			refreshCombo->addItem(ki18n("%1 Hz").subs(rate, 0, 'f', 1).toString(), rate);
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
