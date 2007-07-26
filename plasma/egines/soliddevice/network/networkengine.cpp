/*
 *   Copyright (C) 2007 Percy Leonhardt <percy@eris23.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "networkengine.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <QMap>
#include <QDir>
#include <QTimer>
#include <QRegExp>
#include <QStringList>

#include <KDebug>
#include <KLocale>
#include <kio/global.h>
#include <plasma/datacontainer.h>

// #include "config.h"

// #ifdef HAVE_LIBIW
#include <iwlib.h>
// #else
// #include <net/if.h>
// #endif

#define RTF_GATEWAY   0x0002
#define SYSPATH       "/sys/class/net/"
#define PROCROUTE     "/proc/net/route"

NetworkEngine::NetworkEngine(QObject* parent, const QStringList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args)
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateNetworkData()));
}

NetworkEngine::~NetworkEngine()
{
}

bool NetworkEngine::sourceRequested(const QString &name)
{
    QDir interfaceDir( SYSPATH + name );
    if ( !interfaceDir.exists() )
    {
        return false;
    }

    if ( !m_timer->isActive() )
    {
        m_timer->start(1000);
    }
    return true;
}

void NetworkEngine::updateNetworkData()
{
    DataEngine::SourceDict sources = sourceDict();
    DataEngine::SourceDict::iterator it = sources.begin();
    while ( it != sources.end() )
    {
        QString ifName = it.key();
        Plasma::DataContainer* source = it.value();
        QDir interfaceDir( SYSPATH + ifName );
        if ( !interfaceDir.exists() )
        {
            // remove the source as the interface is no longer available
            removeSource( ifName );
            continue;
        }

        // Check if it is a wireless interface.
        if ( QFile::exists( SYSPATH + ifName + "/wireless" ) )
        {
            source->setData( I18N_NOOP( "Wireless" ), true );
        }
        else
        {
            source->setData( I18N_NOOP( "Wireless" ), false );
        }

        unsigned int carrier = 0;
        if ( !readNumberFromFile( SYSPATH + ifName + "/carrier", carrier ) ||
             carrier == 0 )
        {
            // The interface is there but not useable.
            source->setData( I18N_NOOP( "Running" ), false );
        }
        else
        {
            source->setData( I18N_NOOP( "Running" ), true );

            // Update the interface.
            updateInterfaceData( ifName, source );
            if ( source->data()["Wireless"] == true )
            {
                updateWirelessData( ifName, source );
            }
        }
        ++it;
    }

    checkForUpdates();
}

bool NetworkEngine::readNumberFromFile( const QString &fileName, unsigned int &value )
{
    FILE* file = fopen( fileName.toLatin1().constData(), "r" );
    if ( file != NULL )
    {
        if ( fscanf( file, "%ul", &value ) > 0 )
        {
            fclose( file );
            return true;
        }
        fclose( file );
    }

    return false;
}

bool NetworkEngine::readStringFromFile( const QString &fileName, QString &string )
{
    char buffer[64];
    FILE* file = fopen( fileName.toLatin1().constData(), "r" );
    if ( file != NULL )
    {
        if ( fscanf( file, "%s", buffer ) > 0 )
        {
            fclose( file );
            string = buffer;
            return true;
        }
        fclose( file );
    }

    return false;
}

void NetworkEngine::updateInterfaceData( const QString &ifName, Plasma::DataContainer *source )
{
    QString ifFolder = SYSPATH + ifName + "/";

    unsigned int value = 0;
    if ( readNumberFromFile( ifFolder + "statistics/rx_packets", value ) )
    {
        source->setData( I18N_NOOP( "RX packets" ), value );
    }
    if ( readNumberFromFile( ifFolder + "statistics/tx_packets", value ) )
    {
        source->setData( I18N_NOOP( "TX packets" ), value );
    }
    if ( readNumberFromFile( ifFolder + "statistics/rx_bytes", value ) )
    {
        source->setData( I18N_NOOP( "RX bytes" ), value );
    }
    if ( readNumberFromFile( ifFolder + "statistics/tx_bytes", value ) )
    {
        source->setData( I18N_NOOP( "TX bytes" ), value );
    }
    if ( readNumberFromFile( ifFolder + "/type", value ) &&
         value == 512 )
    {
        source->setData( I18N_NOOP( "Type" ), "PPP" );
    }
    else
    {
        source->setData( I18N_NOOP( "Type" ), "Ethernet" );
        QString hwAddress;
        if ( readStringFromFile( ifFolder + "address", hwAddress ) )
        {
            source->setData( I18N_NOOP( "HW addr" ), hwAddress );
        }
    }

    // for the default gateway we use the proc filesystem
    QFile routeFile( PROCROUTE );
    if ( routeFile.open( QIODevice::ReadOnly ) )
    {
        QString routeData( routeFile.readAll().data() );
        QStringList routeEntries = routeData.split( "\n" );
        foreach ( QString entry, routeEntries )
        {
            QRegExp regExp( ".*\\s+[\\w\\d]{8}\\s+([\\w\\d]{8})\\s+(\\d{4})" );
            if (   ( regExp.indexIn( entry ) > -1 )
                && ( regExp.cap( 2 ).toUInt() & RTF_GATEWAY ) )
            {
                bool ok;
                struct in_addr in;
                in.s_addr = regExp.cap( 1 ).toULong( &ok, 16 );
                source->setData( I18N_NOOP( "Gateway" ), inet_ntoa( in ) );
                break;
            }
        }
        routeFile.close();
    }

    // use ioctls for the rest
    int fd;
    struct ifreq ifr;
    if ( ( fd = socket(AF_INET, SOCK_DGRAM, 0) ) > -1 )
    {
        strcpy( ifr.ifr_name, ifName.toLatin1().constData() );
        ifr.ifr_addr.sa_family = AF_INET;
        if ( ioctl( fd, SIOCGIFADDR, &ifr ) > -1 )
        {
            source->setData( I18N_NOOP( "IP" ), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr) );
        }
        if ( ioctl( fd, SIOCGIFBRDADDR, &ifr ) > -1 )
        {
            source->setData( I18N_NOOP( "Broadcast" ), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_broadaddr)->sin_addr) );
        }
        if ( ioctl( fd, SIOCGIFNETMASK, &ifr ) > -1 )
        {
            source->setData( I18N_NOOP( "Subnet mask" ), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_netmask)->sin_addr) );
        }
        if (   source->data()[ "Type" ] == "PPP"
            && ioctl( fd, SIOCGIFDSTADDR, &ifr) > -1 )
        {
            source->setData( I18N_NOOP( "PTP" ), inet_ntoa(((struct sockaddr_in*)&ifr.ifr_dstaddr)->sin_addr) );
        }
        close( fd );
    }
}

void NetworkEngine::updateWirelessData( const QString &ifName, Plasma::DataContainer *source )
{
    QString wirelessFolder = SYSPATH + ifName + "/wireless/";

    unsigned int link = 0;
    if ( readNumberFromFile( wirelessFolder + "link", link ) )
    {
        source->setData( I18N_NOOP( "Link quality" ), link );
    }

// #ifdef HAVE_LIBIW
    // The following code was taken from iwconfig.c and iwlib.c.
    int fd;
    if ( ( fd = iw_sockets_open() ) > 0 )
    {
        struct iwreq wrq;
        char buffer[128];
        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWFREQ, &wrq ) >= 0 )
        {
            int channel = -1;
            double freq = iw_freq2float( &( wrq.u.freq ) );
            struct iw_range range;
            if( iw_get_range_info( fd, ifName.toLatin1().constData(), &range ) >= 0 )
            {
                if ( freq < KILO )
                {
                    channel = iw_channel_to_freq( (int) freq, &freq, &range );
                }
                else
                {
                    channel = iw_freq_to_channel( freq, &range );
                }
                iw_print_freq_value( buffer, sizeof( buffer ), freq );
                source->setData( I18N_NOOP( "Frequency" ), buffer );
                source->setData( I18N_NOOP( "Channel" ), channel );
            }
        }

        char essid[IW_ESSID_MAX_SIZE + 1];
        memset( essid, 0, IW_ESSID_MAX_SIZE + 1 );
        wrq.u.essid.pointer = (caddr_t) essid;
        wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
        wrq.u.essid.flags = 0;
        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWESSID, &wrq ) >= 0 )
        {
            if ( wrq.u.data.flags > 0 )
            {
                source->setData( I18N_NOOP( "ESSID" ), essid );
            }
            else
            {
                source->setData( I18N_NOOP( "ESSID" ), "any" );
            }
        }

        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWAP, &wrq ) >= 0 )
        {
            char ap_addr[128];
            iw_ether_ntop( (const ether_addr*) wrq.u.ap_addr.sa_data, ap_addr);
            source->setData( I18N_NOOP( "Accesspoint" ),ap_addr );
        }

        memset( essid, 0, IW_ESSID_MAX_SIZE + 1 );
        wrq.u.essid.pointer = (caddr_t) essid;
        wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
        wrq.u.essid.flags = 0;
        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWNICKN, &wrq ) >= 0 )
        {
            if ( wrq.u.data.length > 1 )
            {
                source->setData( I18N_NOOP( "Nickname" ), essid );
            }
            else
            {
                source->setData( I18N_NOOP( "Nickname" ), "" );
            }
        }

        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWRATE, &wrq ) >= 0 )
        {
            iwparam bitrate;
            memcpy (&(bitrate), &(wrq.u.bitrate), sizeof (iwparam));
            iw_print_bitrate( buffer, sizeof( buffer ), wrq.u.bitrate.value );
            source->setData( I18N_NOOP( "Bitrate" ), buffer );
        }

        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWMODE, &wrq ) >= 0 )
        {
            int mode = wrq.u.mode;
            if ( mode < IW_NUM_OPER_MODE && mode >= 0 )
            {
                source->setData( I18N_NOOP( "Mode" ), iw_operation_mode[mode] );
            }
            else
            {
                source->setData( I18N_NOOP( "Mode" ), "" );
            }
        }

        unsigned char key[IW_ENCODING_TOKEN_MAX];
        wrq.u.data.pointer = (caddr_t) key;
        wrq.u.data.length = IW_ENCODING_TOKEN_MAX;
        wrq.u.data.flags = 0;
        if ( iw_get_ext( fd, ifName.toLatin1().constData(), SIOCGIWENCODE, &wrq ) >= 0 )
        {
            if ( ( wrq.u.data.flags & IW_ENCODE_DISABLED ) || ( wrq.u.data.length == 0 ) )
            {
                source->setData( I18N_NOOP( "Encryption" ), false );
            }
            else
            {
                source->setData( I18N_NOOP( "Encryption" ), true );
            }
        }
        else
        {
            source->setData( I18N_NOOP( "Encryption" ), false );
        }
        close( fd );
    }
// #endif
}

#include "networkengine.moc"
