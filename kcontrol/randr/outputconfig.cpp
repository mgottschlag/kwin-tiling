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

OutputConfig::OutputConfig(QWidget* parent, RandROutput* output, OutputConfigList preceding, bool unified)
	: QWidget(parent)
	, precedingOutputConfigs( preceding )
{
	m_output = output;
	m_unified = unified;
	Q_ASSERT(output);

	setupUi(this);

	// connect signals
	connect(positionCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(positionComboChanged(int)));
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(updateRateList(int)));
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(updatePositionList()));
	connect(sizeCombo, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(updateRotationList()));
	connect(m_output, SIGNAL(outputChanged(RROutput,int)),
	        this,     SLOT(outputChanged(RROutput,int)));
		  
	load();

	connect(sizeCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(refreshCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(orientationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(positionCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(positionOutputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	connect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));

	connect(sizeCombo,    SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateView()));
	connect(orientationCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateView()));
	connect(positionCombo,    SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateView()));
	connect(positionOutputCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(updateView()));
	connect(absolutePosX, SIGNAL(valueChanged(int)), this, SIGNAL(updateView()));
	connect(absolutePosY, SIGNAL(valueChanged(int)), this, SIGNAL(updateView()));
	// make sure to update option for relative position when other outputs get enabled/disabled
	foreach( OutputConfig* config, precedingOutputConfigs )
		connect( config, SIGNAL(updateView()), this, SLOT(updatePositionList()));

	updatePositionListTimer.setSingleShot( true );
	connect( &updatePositionListTimer, SIGNAL(timeout()), SLOT(updatePositionListDelayed()));

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
	if( !isActive())
		return QPoint();
	int index = positionCombo->currentIndex();
	if((Relation)positionCombo->itemData(index).toInt() == Absolute)
		return QPoint(absolutePosX->value(), absolutePosY->value());
	
	foreach(OutputConfig *config, precedingOutputConfigs) {
		if( config->output()->id()
			== positionOutputCombo->itemData( positionOutputCombo->currentIndex()).toUInt()) {
			QPoint pos = config->position();
			switch( (Relation)positionCombo->itemData(index).toInt()) {
				case LeftOf:
					return QPoint( pos.x() - resolution().width(), pos.y());
				case RightOf:
					return QPoint( pos.x() + config->resolution().width(), pos.y());
				case Over:
					return QPoint( pos.x(), pos.y() - resolution().height());
				case Under:
					return QPoint( pos.x(), pos.y() + config->resolution().height());
				case SameAs:
					return pos;
				default:
					abort();
			}
		}
	}
	return QPoint(0, 0);
}

QSize OutputConfig::resolution(void) const
{
	if( sizeCombo->count() == 0 )
		return QSize();
	return sizeCombo->itemData(sizeCombo->currentIndex()).toSize();
}

QRect OutputConfig::rect() const
{
	return QRect( position(), resolution());
}

bool OutputConfig::isActive() const
{
        return sizeCombo->count() != 0 && !resolution().isEmpty();
}

float OutputConfig::refreshRate(void) const
{
	if( !isActive())
		return 0;
	float rate = float(refreshCombo->itemData(refreshCombo->currentIndex()).toDouble());
	if(rate == 0.0f) {
		RateList rates = m_output->refreshRates(resolution());
		if (!rates.isEmpty()) {
			return rates.first();
		}
	}
	return rate;
}

int OutputConfig::rotation(void) const
{
	if( !isActive())
		return 0;
	return orientationCombo->itemData(orientationCombo->currentIndex()).toInt();
}

bool OutputConfig::hasPendingChanges( const QPoint& normalizePos ) const
{
	if (m_output->rect().translated( -normalizePos ) != QRect(position(), resolution())) {
		return true;
	}
	else if (m_output->rotation() != rotation()) {
		return true;
	}
	else if (m_output->refreshRate() != refreshRate()) {
		return true;
	}
	return false;
}

void OutputConfig::setUnifyOutput(bool unified)
{
	m_unified = unified;
	updatePositionListTimer.start( 0 );
}

void OutputConfig::outputChanged(RROutput output, int changes)
{
	Q_ASSERT(m_output->id() == output); Q_UNUSED(output);
	kDebug() << "Output" << m_output->name() << "changed. ( mask =" << QString::number(changes) << ")";
	
	disconnect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	disconnect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	if(changes & RandR::ChangeOutputs) {
		kDebug() << "Outputs changed.";
	}
	
	if(changes & RandR::ChangeCrtc) {
		kDebug() << "Output CRTC changed.";
		
		updateSizeList();
		updateRateList();
		updateRotationList();
	}
	
	if(changes & RandR::ChangeRect) {
		QRect r = m_output->rect();
		kDebug() << "Output rect changed:" << r;
		updatePositionList();
	}
	
	if(changes & RandR::ChangeRotation) {
		kDebug() << "Output rotation changed.";
		updateRotationList();
	}
	
	if(changes & RandR::ChangeConnection) {
		kDebug() << "Output connection status changed.";
		setEnabled(m_output->isConnected());
		emit connectedChanged(m_output->isConnected());
	}
	
	if(changes & RandR::ChangeRate) {
		kDebug() << "Output rate changed.";
		updateRateList();
	}
	
	if(changes & RandR::ChangeMode) {
		kDebug() << "Output mode changed.";
		updateSizeList();
		
		// This NEEDS to be fixed..
		//QSize modeSize = m_output->screen()->mode(m_output->mode()).size();
		QSize modeSize = m_output->mode().size();
		updateRateList(sizeCombo->findData(modeSize));
	}
	connect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	connect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
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
	kDebug() << "Loading output configuration for" << m_output->name();
	setEnabled( m_output->isConnected() );

	orientationCombo->clear();

	if (!m_output->isConnected())
		return;

	/* Mode size configuration */
	updateSizeList();
	
	/* Output rotation and relative position */
	updateRotationList();
	updatePositionList();
	
	emit updateView();
}

void OutputConfig::setConfigDirty(void)
{
	m_changed = true;
	emit optionChanged();
}

bool OutputConfig::isRelativeTo( QRect rect, QRect to, Relation rel )
{
	switch( rel ) {
		case LeftOf:
			return rect.x() + rect.width() == to.x() && rect.y() == to.y();
		case RightOf:
			return rect.x() == to.x() + to.width() && rect.y() == to.y();
		case Over:
			return rect.x() == to.x() && rect.y() + rect.height() == to.y();
		case Under:
			return rect.x() == to.x() && rect.y() == to.y() + to.height();
		case SameAs:
			return rect.topLeft() == to.topLeft();
		case Absolute:
		default:
			return false;
	}
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
		
		disconnect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
		disconnect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
		absolutePosX->setValue(posX);
		absolutePosY->setValue(posY);
		connect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
		connect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	}
}

void OutputConfig::updatePositionList(void)
{
	// Delay because
	// a) this is an optimization
	// b) this can be called in the middle of changing configuration and can
	//    lead to the comboboxes being setup to wrong values
	updatePositionListTimer.start( 0 );
}

void OutputConfig::updatePositionListDelayed()
{
	positionLabel->setVisible(true);
	positionCombo->setVisible(true);
	positionOutputCombo->setVisible(true);
	absolutePosX->setVisible(true);
	absolutePosY->setVisible(true);

	disconnect(positionCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	disconnect(positionOutputCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	disconnect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	disconnect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));

	bool enable = !resolution().isEmpty();
	positionCombo->setEnabled( enable );
	positionLabel->setEnabled( enable );
	positionOutputCombo->setEnabled( enable );
	absolutePosX->setEnabled( enable );
	absolutePosY->setEnabled( enable );

	positionCombo->clear();
	positionOutputCombo->clear();

	OutputConfigList cleanList;
	foreach(OutputConfig *config, precedingOutputConfigs) {
		if( config->resolution().isEmpty()) {
			continue; // ignore disabled outputs
		}
		cleanList.append(config);

	}
	Relation rel = Absolute;
	// FIXME: get default value from KConfig
	if (m_unified && !cleanList.isEmpty()) {
		positionCombo->addItem(OutputConfig::positionName(OutputConfig::SameAs), OutputConfig::SameAs);
	} else {
		for(int i = -1; i < 5; i++)
			positionCombo->addItem(OutputConfig::positionName((Relation)i), i);
	}
	
	int index = positionCombo->findData((int)rel);
	if(index != -1) {
		positionCombo->setCurrentIndex(index);
	} else {
		positionCombo->setCurrentIndex(positionCombo->findData((int)OutputConfig::SameAs));
	}

	/* Relative Output Name Configuration */
	foreach(OutputConfig *config, cleanList) {
		RandROutput* output = config->output();
		positionOutputCombo->addItem(QIcon(output->icon()), output->name(), (int)output->id());
		if (!m_unified) {
			for( int rel = -1; rel < 5; ++rel ) {
				if( isRelativeTo( m_output->rect(), QRect( config->position(), config->resolution()), (Relation) rel )) {
					positionCombo->setCurrentIndex( positionCombo->findData( rel ));
				}
			}
		}
	}
        if( positionOutputCombo->count() == 0 ) {
            positionOutputCombo->setEnabled( false );
            while( positionCombo->count() > 1 ) // keep only 'Absolute'
                positionCombo->removeItem( positionCombo->count() - 1 );
        }

	if (m_unified) {
		positionLabel->setEnabled(false);
		positionCombo->setEnabled(false);
		positionOutputCombo->setEnabled(false);
		absolutePosX->setEnabled(false);
		absolutePosY->setEnabled(false);
	}
	// FIXME: get this from Kconfig again
	/*if(m_output->relation(0) != m_output) {
		index = positionOutputCombo->findData((int)m_output->relation(0)->id());
		if(index != -1)
			positionOutputCombo->setCurrentIndex(index);
	}*/

	connect(positionCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(positionOutputCombo,    SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigDirty()));
	connect(absolutePosX, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
	connect(absolutePosY, SIGNAL(valueChanged(int)), this, SLOT(setConfigDirty()));
}

void OutputConfig::updateRotationList(void)
{
	Q_FOREACH(OutputConfig *config, precedingOutputConfigs) {
		if (m_unified) {
			connect(config->orientationCombo, SIGNAL(activated(int)), orientationCombo, SLOT(setCurrentIndex(int)));
			connect(orientationCombo, SIGNAL(activated(int)), config->orientationCombo, SLOT(setCurrentIndex(int)));
		} else {
			disconnect(config->orientationCombo, SIGNAL(activated(int)), orientationCombo, SLOT(setCurrentIndex(int)));
			disconnect(orientationCombo, SIGNAL(activated(int)), config->orientationCombo, SLOT(setCurrentIndex(int)));
		}
	}

	bool enable = !resolution().isEmpty();
	orientationCombo->setEnabled( enable );
	orientationLabel->setEnabled( enable );
	orientationCombo->clear();
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
	if (m_unified) {
		sizes = m_output->screen()->unifiedSizes();
	}
	Q_FOREACH(OutputConfig *config, precedingOutputConfigs) {
		if (m_unified) {
			connect(config->sizeCombo, SIGNAL(activated(int)), sizeCombo, SLOT(setCurrentIndex(int)));
			connect(sizeCombo, SIGNAL(activated(int)), config->sizeCombo, SLOT(setCurrentIndex(int)));
		} else {
			disconnect(config->sizeCombo, SIGNAL(activated(int)), sizeCombo, SLOT(setCurrentIndex(int)));
			disconnect(sizeCombo, SIGNAL(activated(int)), config->sizeCombo, SLOT(setCurrentIndex(int)));
		}
	}
	RandRMode preferredMode = m_output->preferredMode();
	sizeCombo->clear();
	sizeCombo->addItem( i18nc("Screen size", "Disabled"), QSize(0, 0) );
	
	foreach (const QSize &s, sizes) {
		QString sizeDesc = QString("%1x%2").arg(s.width()).arg(s.height());
		if (preferredMode.isValid() && s == preferredMode.size()) {
			sizeDesc = i18nc("Automatic screen size (native resolution)",
			                 "%1 (Auto)", sizeDesc);
		}
		sizeCombo->addItem( sizeDesc, s );
	}
	
	int index = -1;

    // if output is rotated 90 or 270 degrees, swap width and height before searching in combobox data
    // otherwise 90 or 270 degrees rotated outputs will be set as "Disabled" in GUI
	if (m_output->rotation() == RandR::Rotate90 || m_output->rotation() == RandR::Rotate270)
		index = sizeCombo->findData( QSize(m_output->rect().height(), m_output->rect().width()) );
	else
		index = sizeCombo->findData( m_output->rect().size() );

	if (index != -1) {
		sizeCombo->setCurrentIndex( index );
	} else if (!sizes.isEmpty()) {
        kDebug() << "Output size cannot be matched! fallbacking to the first size";
		sizeCombo->setCurrentIndex(index = sizeCombo->findData(sizes.first()));
	}

	index = refreshCombo->findData(m_output->refreshRate());
	if (index != -1)
		refreshCombo->setCurrentIndex(index);
}

void OutputConfig::updateRateList(int resolutionIndex)
{
	QSize resolution = sizeCombo->itemData(resolutionIndex).toSize();
	if((resolution == QSize(0, 0)) || !resolution.isValid()) {
		refreshCombo->setEnabled(false);
		rateLabel->setEnabled(false);
		return;
	}
	
	ModeList modeList = m_output->modes();
	
	refreshCombo->clear();
	refreshCombo->addItem(i18nc("Automatic refresh rate configuration", "Auto"), 0.0f);
	refreshCombo->setEnabled(true);
	rateLabel->setEnabled(true);
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
