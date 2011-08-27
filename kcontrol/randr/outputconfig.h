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

#ifndef __OUTPUTCONFIG_H__
#define __OUTPUTCONFIG_H__

#include <QWidget>
#include <QTextStream>
#include <QTimer>
#include "ui_outputconfigbase.h"
#include "randr.h"
#include "randroutput.h"

class RandROutput;

class OutputConfig;
typedef QList<OutputConfig*> OutputConfigList;

class OutputConfig : public QWidget, public Ui::OutputConfigBase 
{
	Q_OBJECT
public:
	OutputConfig(QWidget *parent, RandROutput *output, OutputConfigList preceding, bool unified);
	~OutputConfig();
	
	/** Enumeration describing two related outputs (i.e. VGA LeftOf TMDS) */
	enum Relation {
		Absolute = -1,
		SameAs = 0,
		LeftOf = 1,
		RightOf,
		Over,
		Under
	};
	// NOTE: I'd love to have used Above and Below but Xlib already defines them
	// and that confuses GCC.

	bool isActive() const;
	QPoint position(void) const;
	QSize resolution(void) const;
	QRect rect() const;
	float refreshRate(void) const;
	int rotation(void) const;
	
	static QString positionName(Relation position);
	RandROutput *output(void) const;

	bool hasPendingChanges( const QPoint& normalizePos ) const;
	void setUnifyOutput(bool unified);
public slots:
	void load();
	void updateSizeList(void);

protected slots:
	void setConfigDirty(void);
	
	void updatePositionList(void);
	void updatePositionListDelayed(void);
	void updateRotationList(void);
	void updateRateList(void);
	void updateRateList(int resolutionIndex);
	
	void positionComboChanged(int item);	
	void outputChanged(RROutput output, int changed);

signals:
	void updateView();
	void optionChanged();
	void connectedChanged(bool);


private:
	static bool isRelativeTo( QRect rect, QRect to, Relation rel );
	int m_changes;
	bool m_changed;
	bool m_unified;
	QPoint m_pos;
	QTimer updatePositionListTimer;
	
	RandROutput *m_output;
	// List of configs shown before this one. Relative positions may be given only
	// relative to these in order to avoid cycles.
	OutputConfigList precedingOutputConfigs;
};


#endif
