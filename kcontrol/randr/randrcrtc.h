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

#ifndef __RANDRCRTC_H__
#define __RANDRCRTC_H__

#include <QObject>
#include <QRect>
#include "randr.h"


class RandRCrtc : public QObject
{
	Q_OBJECT

public:
	RandRCrtc(RandRScreen *parent, RRCrtc id);
	~RandRCrtc();

	RRCrtc id() const;
	int rotations() const;
	int rotation() const;

	void loadSettings(bool notify = false);
	void handleEvent(XRRCrtcChangeNotifyEvent *event);

	RRMode mode() const;
	QRect rect() const;
	float refreshRate() const;

	bool proposeSize(const QSize &s);
	bool proposePosition(const QPoint &p);
	bool proposeRotation(int rotation);
	bool proposeRefreshRate(float rate);
	
	// applying stuff
	bool applyProposed();
	void proposeOriginal();
	void setOriginal(); 
	bool proposedChanged();

	bool addOutput(RROutput output, const QSize &size = QSize());
	bool removeOutput(RROutput output);
	OutputList connectedOutputs() const;

	ModeList modes() const;

signals:
	void crtcChanged(RRCrtc c, int changes);

private:
	RRCrtc m_id;
	RRMode m_currentMode;

	QRect m_proposedRect;
	int m_proposedRotation;
	float m_proposedRate;

	QRect m_originalRect;
	int m_originalRotation;
	float m_originalRate;

	OutputList m_connectedOutputs;
	OutputList m_possibleOutputs;
	int m_rotations;

	int m_currentRotation;
	QRect m_currentRect;
	float m_currentRate;

	RandRScreen *m_screen;
};

#endif
