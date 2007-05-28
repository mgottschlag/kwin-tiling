/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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

#ifndef __RANDRDISPLAY_H__
#define __RANDRDISPLAY_H__

#include <QList>
#include <QWidget>
#include <KConfig>

class RandRScreen;

typedef QList<RandRScreen*> ScreenList;

class RandRDisplay
{
public:
	RandRDisplay();
	~RandRDisplay();
	bool			isValid() const;
	const QString&	errorCode() const;
	const QString&	version() const;

	int		eventBase() const;
	int		screenChangeNotifyEvent() const;
	int		errorBase() const;

	int		screenIndexOfWidget(QWidget* widget);

	int				numScreens() const;
	RandRScreen*	screen(int index);

	void			setCurrentScreen(int index);
	int				currentScreenIndex() const;
	RandRScreen*	currentScreen();

	void	refresh();

	/**
	 * Loads saved settings.
	 *
	 * @param config the KConfig object to load from
	 * @param loadScreens whether to call RandRScreen::load() for each screen
	 * @retuns true if the settings should be applied on KDE startup.
	 */
	bool	loadDisplay(KConfig& config, bool loadScreens = true);
	void	saveDisplay(KConfig& config, bool applyOnStartup, bool syncTrayApp);

	static bool		applyOnStartup(KConfig& config);
	static bool		syncTrayApp(KConfig& config);

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
