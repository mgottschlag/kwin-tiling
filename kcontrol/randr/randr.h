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

#include <qobject.h>
#include <qstringlist.h>
#include <qptrlist.h>

#include <kcmodule.h>
#include <kconfig.h>

#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>

class KTimerDialog;

class RandRScreen : public QObject
{
	Q_OBJECT

public:
	RandRScreen(int screenIndex);
	~RandRScreen();

	void		loadSettings();
	void		setOriginal();

	Status		applyProposed();

	// A return value of -1 means the user did not confirm in time, or cancelled.
	Status		applyProposedAndConfirm();

public slots:
	bool		confirm();

public:
	QString		changedMessage() const;

	bool		changedFromOriginal() const;
	void		proposeOriginal();

	bool		proposedChanged() const;

	static QString	rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	QPixmap	        rotationIcon(int rotation) const;
	QString			currentRotationDescription() const;

	QStringList refreshRates(SizeID size) const;

	QString		refreshRateDirectDescription(int rate) const;
	QString		refreshRateIndirectDescription(SizeID size, int index) const;
	QString		refreshRateDescription(SizeID size, int index) const;

	int			currentRefreshRate() const;
	QString		currentRefreshRateDescription() const;

	int			proposedRefreshRate() const;
	// NOTE this is an index, not a refresh rate in hz!
	void		proposeRefreshRate(int index);

	// Refresh rate hz <==> index conversion
	int			refreshRateHzToIndex(SizeID size, int hz) const;
	int			refreshRateIndexToHz(SizeID size, int index) const;

	int						numSizes() const;
	const XRRScreenSize&	size(int index) const;
	const Rotation			rotations() const;

	int			currentWidth() const;
	int			currentHeight() const;

	Rotation	currentRotation() const;
	SizeID		currentSize() const;

	Rotation	proposedRotation() const;
	void		proposeRotation(Rotation newRotation);

	SizeID		proposedSize() const;
	void		proposeSize(SizeID newSize);

	void		load(KConfig& config);
	void		save(KConfig& config) const;

private:
	XRRScreenConfiguration	*m_config;

	int			m_screen;
	Window		m_root;

	QMemArray<XRRScreenSize>	m_sizes;
	Rotation					m_rotations;

	Rotation	m_originalRotation;
	SizeID		m_originalSize;
	int			m_originalRefreshRate;

	Rotation	m_currentRotation;
	SizeID		m_currentSize;
	int			m_currentRefreshRate;

	Rotation	m_proposedRotation;
	SizeID		m_proposedSize;
	int			m_proposedRefreshRate;

	KTimerDialog*	m_shownDialog;

private slots:
	void		desktopResized();
	void		shownDialogDestroyed();
};

typedef QPtrList<RandRScreen> ScreenList;

class RandRDisplay
{
public:
	RandRDisplay();
	
	bool			isValid() const;
	const QString&	errorCode() const;
	const QString&	version() const;

	int		eventBase() const;
	int		errorBase() const;

	int		screenIndexOfWidget(QWidget* widget);

	int				numScreens() const;
	RandRScreen*	screen(int index);

	void			setCurrentScreen(int index);
	int				currentScreenIndex() const;
	RandRScreen*	currentScreen();

	void	refresh(int screen);

	/**
	 * Loads settings. Loads the settings for individual screens if
	 * loadScreens is true.
	 *
	 * Returns whether the settings should be applied on startup.
	 */
	bool	loadDisplay(KConfig& config, bool loadScreens = true);
	void	saveDisplay(KConfig& config, bool applyOnStartup);

	void	applyProposed(bool confirm = true);

private:
	int				m_numScreens;
	int				m_currentScreenIndex;
	RandRScreen*	m_currentScreen;
	ScreenList		m_screens;

	bool			m_valid;
	QString			m_errorCode;
	QString			m_version;

	int				m_eventBase;
	int				m_errorBase;
};

#endif
