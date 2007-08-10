/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include "randr.h"
#include "layoutmanager.h"
#include "randrscreen.h"
#include "randroutput.h"

#ifdef HAS_RANDR_1_2

LayoutManager::LayoutManager(RandRScreen *screen)
: QObject(screen)
{
	m_screen = screen;
}

LayoutManager::~LayoutManager()
{
}

void LayoutManager::adjustOutputs()
{
	OutputMap activeOutputs;
	
	// get the list of connected outputs
	foreach(RandROutput *output, m_screen->outputs())
		if (output->isActive())
			activeOutputs[output->id()] = output;

	 // if there is only one active output, force it to be at 0x0
	if (activeOutputs.count() == 1)
	{
		RandROutput *output = *activeOutputs.begin();
		output->proposePosition(QPoint(0,0));
		output->applyProposed();
		return;
	}

	// if we have more than one active output, then we have to manage its layout
	if (activeOutputs.count() > 1)
	{
		// TODO implement
	}
}

#endif // HAS_RANDR_1_2
