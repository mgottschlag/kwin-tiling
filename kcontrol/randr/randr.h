/* 
 * Copyright (c) 2002 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __RANDR_H__
#define __RANDR_H__

#include <qstringlist.h>
#include <qptrlist.h>

#include <kcmodule.h>

#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>

class RandRScreen
{
public:
	RandRScreen(int screenIndex, bool requestScreenChangeEvents = false);
	~RandRScreen();
	
	void		loadSettings();
	void		setOriginal();
	
	Status		applyProposed();
	
	// A return value of -1 means the user did not confirm in time, or cancelled.
	Status		applyProposedAndConfirm();
	bool		confirm();
	
	QString		changedMessage();

	bool		changedFromOriginal();
	void		proposeOriginal();
	
	bool		proposedChanged();
	
	static QString	rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	static QPixmap	rotationIcon(int rotation);
	QString		currentRotationDescription();
	
	int			proposedRefreshRateIndex;
	QStringList refreshRates(SizeID size);
	QString		currentRefreshRateDescription();
	void		proposeRefreshRate(QString rateString);
	
	XRRScreenConfiguration	*config;
	
	int			screen;
	Window		root;
	
	QMemArray<XRRScreenSize>	sizes;
	Rotation					rotations;
	
	int			currentWidth();
	int			currentHeight();
	
	Rotation	originalRotation;
	SizeID		originalSize;
	short		originalRefreshRate;
	
	Rotation	currentRotation;
	SizeID		currentSize;
	short		currentRefreshRate;

	Rotation	proposedRotation;
	SizeID		proposedSize;
	short		proposedRefreshRate;
};

typedef QPtrList<RandRScreen> ScreenList;

class RandRDisplay
{
public:
	RandRDisplay(bool requestScreenChangeEvents = false);
	
	bool	isValid() const { return m_valid; };
	QString	errorCode() { return m_errorCode; };

	QString	version() { return m_version; };
	
	inline int	eventBase() const { return m_eventBase; };
	inline int	errorBase() const { return m_errorBase; };
	
	void	setScreen(int index);
	int		widgetScreen(QWidget* widget);
	int		currentScreenIndex() { return m_currentScreenIndex; };
	
	void	refresh();

protected:
	int				m_numScreens;
	int				m_currentScreenIndex;
	RandRScreen*	m_currentScreen;
	ScreenList		m_screens;

private:
	bool			m_valid;
	QString			m_errorCode;
	QString			m_version;
	
	int				m_eventBase;
	int				m_errorBase;
};

#endif
