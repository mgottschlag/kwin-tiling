/***************************************************************************
 *   Copyright (C) 2005 by Olivier Goffart   *
 *   ogoffart@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef SOUND_H
#define SOUND_H

#include <QVector>
#include <QtCore/QCharRef>
#include <kdemacros.h>

/**
@author Olivier Goffart
*/
class KDE_EXPORT Sound{
public:
    Sound();
    ~Sound();

	void load(const QString &filename);
	void save(const QString &filename) const;

	unsigned int size() const
	{
		return data.size();
	}

	inline float at(int pos) const
	{
		return (float)(data.at(pos))/max;
	}

	inline uint fs() const
	{
		return _fs;
	}

	QVector<Q_INT32> data;
	quint32 max;
	uint _fs;
};

#endif
