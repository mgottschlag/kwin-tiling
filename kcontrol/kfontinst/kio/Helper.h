#ifndef __HELPER_H__
#define __HELPER_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

namespace KFI
{
    enum ECommands
    {
        CMD_ENABLE_FONT,
        CMD_DISABLE_FONT,
        CMD_DELETE_DISABLED_FONT,
        CMD_RELOAD_DISABLED_LIST,
        CMD_COPY_FILE,
        CMD_MOVE_FILE,
        CMD_DELETE_FILE,
        CMD_CREATE_DIR,
        CMD_CREATE_AFM,
        CMD_FC_CACHE,
        CMD_ADD_DIR_TO_FONTCONFIG,
        CMD_CFG_DIR_FOR_X,
        CMD_QUIT
    };
}

#endif
