/***************************************************************************
 *   Copyright (C) 2010 by Artur Duque de Souza <asouza@kde.org>           *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

function url() {
    return "http://pastebin.ca/quiet-paste.php";
}

function contentKey() {
    return "content";
}

function setup() {
    provider.addArgument("api", "yhDgkQ9mSjoOTVrTX4XqP4jRDasxZYXX");
    provider.addArgument("description", "");
    provider.addArgument("type", "1");
    provider.addArgument("expiry", "1%20day");
    provider.addArgument("name", "");
}

function handleResultData(data) {
    if (data.search("SUCCESS") == -1) {
        provider.error(data);
        return;
    }
    provider.success(data.replace("SUCCESS:", "http://pastebin.ca/"));
}
