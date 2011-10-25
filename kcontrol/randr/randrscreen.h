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

#ifndef __RANDRSCREEN_H__
#define __RANDRSCREEN_H__

#include "randr.h"

#include <QX11Info>
#include <QObject>
#include <QMap>

class QSize;
class QAction;
class KConfig;

class RandRScreen : public QObject
{
	Q_OBJECT

public:
	RandRScreen(int screenIndex);
	~RandRScreen();

	int index() const;

	XRRScreenResources* resources() const;
	Window rootWindow() const;

	QSize minSize() const;
	QSize maxSize() const;

	void loadSettings(bool notify = false);

	void handleEvent(XRRScreenChangeNotifyEvent* event);
	void handleRandREvent(XRRNotifyEvent* event);

	CrtcMap  crtcs() const;
	RandRCrtc *crtc(RRCrtc id) const;
	
	OutputMap outputs() const;
	RandROutput *output(RROutput id) const;

#ifdef HAS_RANDR_1_3
	void setPrimaryOutput(RandROutput* output);
	RandROutput* primaryOutput();

	void proposePrimaryOutput(RandROutput* output);
#endif

	ModeMap modes() const;
	RandRMode mode(RRMode id) const;

	bool adjustSize(const QRect &minimumSize = QRect(0,0,0,0));
	bool setSize(const QSize &s);

	/**
	 * Return the number of connected outputs
	 */
	int connectedCount() const;

	/**
	 * Return the number of active outputs
	 */
	int activeCount() const;

	bool outputsUnified() const;
	void setOutputsUnified(bool unified);

	int unifiedRotations() const;
	SizeList unifiedSizes() const;

	QRect rect() const;

	bool applyProposed(bool confirm);

	void load(KConfig &config, bool skipOutputs = false);
	void save(KConfig &config);
	QStringList startupCommands() const;

public slots:
	void slotUnifyOutputs(bool unify);
	void slotResizeUnified(QAction *action);
	void slotRotateUnified(QAction *action);

	void slotOutputChanged(RROutput id, int changes);

	void save();
	void load();

signals:
	void configChanged();

protected slots:
	void unifyOutputs();

private:
	int m_index;
	QSize m_minSize;
	QSize m_maxSize;
	QRect m_rect;

	bool m_outputsUnified;
	QRect m_unifiedRect;
	int m_unifiedRotation;

	int m_connectedCount;
	int m_activeCount;

#ifdef HAS_RANDR_1_3
	RandROutput* m_originalPrimaryOutput;
	RandROutput* m_proposedPrimaryOutput;
#endif //HAS_RANDR_1_3

	XRRScreenResources* m_resources;

	CrtcMap m_crtcs;
	OutputMap m_outputs;
	ModeMap m_modes;
		
};

#endif
// vim:noet:sts=8:sw=8:
