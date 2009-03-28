/*
 * wicd-defines.h
 *
 *  Created on: 4-nov-2008
 *      Author: drf
 */

#ifndef WICDDEFINES_H
#define WICDDEFINES_H

// All Wicd Definitions will be stored in here

#define WICD_DBUS_SERVICE "org.wicd.daemon"
#define WICD_DBUS_PATH "/org/wicd/daemon"
#define WICD_DAEMON_DBUS_INTERFACE "org.wicd.daemon"
#define WICD_CONFIG_DBUS_INTERFACE "org.wicd.daemon.config"
#define WICD_WIRED_DBUS_INTERFACE "org.wicd.daemon.wired"
#define WICD_WIRELESS_DBUS_INTERFACE "org.wicd.daemon.wireless"

namespace Wicd
{
enum ConnectionStatus {
    NOT_CONNECTED = 0,
    CONNECTING = 1,
    WIRELESS = 2,
    WIRED = 3,
    SUSPENDED = 4,
    Unknown = 16
};
}

#endif /* WICDDEFINES_H */
