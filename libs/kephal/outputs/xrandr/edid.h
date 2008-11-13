/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef KEPHAL_EDID_H
#define KEPHAL_EDID_H

#define EDID_HEADER "\x00\xff\xff\xff\xff\xff\xff\x00"
#define EDID_HEADER_SIZE 8
#define EDID_TEST_HEADER(x) (memcmp(x, EDID_HEADER, EDID_HEADER_SIZE) == 0)

#define EDID_VENDOR_START 8
#define EDID_VENDOR_1(x) ('@' + ((x[EDID_VENDOR_START] & 0x7c) >> 2))
#define EDID_VENDOR_2(x) ('@' + ((x[EDID_VENDOR_START] & 0x03) << 3) + ((x[EDID_VENDOR_START + 1] & 0xe0) >> 5))
#define EDID_VENDOR_3(x) ('@' + (x[EDID_VENDOR_START + 1] & 0x1f))

#define EDID_PRODUCT_ID_START (EDID_VENDOR_START + 2)
#define EDID_PRODUCT_ID(x) (x[EDID_PRODUCT_ID_START] | (x[EDID_PRODUCT_ID_START + 1] << 8))

#define EDID_SERIAL_NUMBER_START (EDID_PRODUCT_ID_START + 2)
#define EDID_SERIAL_NUMBER(x) (x[EDID_SERIAL_NUMBER_START] | (x[EDID_SERIAL_NUMBER_START + 1] << 8) | (x[EDID_SERIAL_NUMBER_START + 2] << 16) | (x[EDID_SERIAL_NUMBER_START + 3] << 24))

#endif // KEPHAL_EDID_H

