/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include "randr.h"
#include <KIconLoader>

bool RandR::has_1_2 = false;
bool RandR::has_1_3 = false;
Time RandR::timestamp = 0;

QString RandR::rotationName(int rotation, bool pastTense, bool capitalised)
{
	if (!pastTense)
		switch (rotation) {
			case RR_Rotate_0:
				return i18n("No Rotation");
			case RR_Rotate_90:
				return i18n("Left (90 degrees)");
			case RR_Rotate_180:
				return i18n("Upside-Down (180 degrees)");
			case RR_Rotate_270:
				return i18n("Right (270 degrees)");
			case RR_Reflect_X:
				return i18n("Mirror Horizontally");
			case RR_Reflect_Y:
				return i18n("Mirror Vertically");
			default:
				return i18n("Unknown Orientation");
		}

	switch (rotation) {
		case RR_Rotate_0:
			return i18n("Not Rotated");
		case RR_Rotate_90:
			return i18n("Rotated 90 Degrees Counterclockwise");
		case RR_Rotate_180:
			return i18n("Rotated 180 Degrees Counterclockwise");
		case RR_Rotate_270:
			return i18n("Rotated 270 Degrees Counterclockwise");
		default:
			if (rotation & RR_Reflect_X)
				if (rotation & RR_Reflect_Y)
					if (capitalised)
						return i18n("Mirrored Horizontally And Vertically");
					else
						return i18n("mirrored horizontally and vertically");
				else
					if (capitalised)
						return i18n("Mirrored Horizontally");
					else
						return i18n("mirrored horizontally");
			else if (rotation & RR_Reflect_Y)
				if (capitalised)
					return i18n("Mirrored Vertically");
				else
					return i18n("mirrored vertically");
			else
				if (capitalised)
					return i18n("Unknown Orientation");
				else
					return i18n("unknown orientation");
	}
}

QPixmap RandR::rotationIcon(int rotation, int currentRotation)
{
	// Adjust icons for current screen orientation
	if (!(currentRotation & RR_Rotate_0) && rotation & (RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270)) {
		int currentAngle = currentRotation & (RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270);
		switch (currentAngle) {
			case RR_Rotate_90:
				rotation <<= 3;
				break;
			case RR_Rotate_180:
				rotation <<= 2;
				break;
			case RR_Rotate_270:
				rotation <<= 1;
				break;
		}

		// Fix overflow
		if (rotation > RR_Rotate_270) {
			rotation >>= 4;
		}
	}

	switch (rotation) {
		case RR_Rotate_0:
			return SmallIcon("go-up");
		case RR_Rotate_90:
			return SmallIcon("go-previous");
		case RR_Rotate_180:
			return SmallIcon("go-down");
		case RR_Rotate_270:
			return SmallIcon("go-next");
		case RR_Reflect_X:
			return SmallIcon("object-flip-horizontal");
		case RR_Reflect_Y:
			return SmallIcon("object-flip-vertical");
		default:
			return SmallIcon("process-stop");
	}
}

bool RandR::confirm(const QRect &rect)
{
	Q_UNUSED(rect);
	// FIXME remember to put the dialog on the right screen

	KTimerDialog acceptDialog(15000, KTimerDialog::CountDown,
	                          0, "mainKTimerDialog", true,
	                          i18n("Confirm Display Setting Change"),
	                          KTimerDialog::Ok|KTimerDialog::Cancel,
	                          KTimerDialog::Cancel);

	acceptDialog.setButtonGuiItem(KDialog::Ok, KGuiItem(i18n("&Accept Configuration"), "dialog-ok"));
	acceptDialog.setButtonGuiItem(KDialog::Cancel, KGuiItem(i18n("&Revert to Previous Configuration"), "dialog-cancel"));

	QLabel *label = new QLabel(i18n("Your screen configuration has been "
                    "changed to the requested settings. Please indicate whether you wish to keep "
                    "this configuration. In 15 seconds the display will revert to your previous "
                    "settings."), &acceptDialog);
	label->setWordWrap( true );
	acceptDialog.setMainWidget(label);

	//FIXME: this should be changed to use the rect instead of centerOnScreen
	//KDialog::centerOnScreen(&acceptDialog, m_screen);

	return acceptDialog.exec();
}

SizeList RandR::sortSizes(const SizeList &sizes)
{
	int *sizeSort = new int[sizes.count()];
	int numSizes = sizes.count();
	SizeList sorted;

	int i = 0;
	foreach(const QSize &size, sizes)
		sizeSort[i++] = size.width() * size.height();

	for (int j = 0; j < numSizes; j++) 
	{
		int highest = -1, highestIndex = -1;

		for (int i = 0; i < numSizes; i++) 
		{
			if (sizeSort[i] && sizeSort[i] > highest) 
			{
				highest = sizeSort[i];
				highestIndex = i;
			}
		}
		sizeSort[highestIndex] = -1;
		Q_ASSERT(highestIndex != -1);

		sorted.append(sizes[highestIndex]);
	}
	delete [] sizeSort;
    sizeSort = 0L;

	return sorted;
}



