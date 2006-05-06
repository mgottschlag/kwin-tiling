/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "main.h"

#include <iostream>
using namespace std;

#include <QString>
#include <QStringList>

#include <kinstance.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <kdehw/devicemanager.h>
#include <kdehw/device.h>
#include <kdehw/volume.h>

#include <kjob.h>

static const char appName[] = "solidshell";
static const char programName[] = I18N_NOOP("solidshell");

static const char description[] = I18N_NOOP("KDE tool for querying and controlling your hardware from the command line");

static const char version[] = "0.1";

static const KCmdLineOptions options[] =
{
   { "commands", I18N_NOOP("Show available commands by domains"), 0},
   { "+domain", I18N_NOOP("Domain (see --commands)"), 0},
   { "+command", I18N_NOOP("Command (see --commands)"), 0},
   { "+[arg(s)]", I18N_NOOP("Arguments for command"), 0},
   KCmdLineLastOption
};

std::ostream &operator<<( std::ostream &out, const QString &msg )
{
    return ( out << msg.toLocal8Bit().constData() );
}

std::ostream &operator<<( std::ostream &out, const QMap<QString,QVariant> &properties )
{
    foreach( QString key, properties.keys() )
    {
        out << "  " << key << " = ";

        QVariant property = properties[key];

        switch ( property.type() )
        {
        case QVariant::StringList:
        {
            out << "{";

            QStringList list = property.toStringList();

            QStringList::ConstIterator it = list.begin();
            QStringList::ConstIterator end = list.end();

            for ( ; it!=end; ++it )
            {
                out << "'" << *it << "'";

                if ( it+1!=end )
                {
                    out << ", ";
                }
            }

            out << "}  (string list)" << endl;
            break;
        }
        case QVariant::Bool:
            out << ( property.toBool()?"true":"false" ) << "  (bool)" << endl;
            break;
        case QVariant::Int:
            out << property.toString()
                << "  (0x" << QString::number( property.toInt(), 16 ) << ")  (int)" << endl;
            break;
        default:
            out << "'" << property.toString() << "'  (string)" << endl;
            break;
        }
    }

    return out;
}

void checkArgumentCount( int min, int max )
{
    int count = KCmdLineArgs::parsedArgs()->count();

    if ( count < min )
    {
        cerr << i18n( "Syntax Error: Not enough arguments" ) << endl;
        ::exit( 1 );
    }

    if ( ( max > 0 ) && ( count > max ) )
    {
        cerr << i18n( "Syntax Error: Too many arguments" ) << endl;
        ::exit( 1 );
    }
}

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, appName, programName, description, version, false);

  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::addTempFileOption();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KInstance instance( appName );

  if ( args->isSet( "commands" ) )
  {
      KCmdLineArgs::enable_i18n();

      cout << endl << i18n( "Syntax:" ) << endl << endl;

      cout << "  solidshell hardware list [details]" << endl;
      cout << i18n( "             # List the hardware available in the system.\n"
                    "             # If the details option is specified, the device properties are\n"
                    "             # listed (be careful, property names are backend dependent),\n"
                    "             # otherwise only device UDIs are listed.\n" ) << endl;

      cout << "  solidshell hardware properties 'udi'" << endl;

      cout << "  solidshell hardware query 'predicate' ['parentUdi']" << endl;

      cout << "  solidshell hardware mount 'udi'" << endl;
      cout << "  solidshell hardware unmount 'udi'" << endl;
      cout << "  solidshell hardware eject 'udi'" << endl;

      return 0;
  }

  return SolidShell::doIt() ? 0 : 1;
}

bool SolidShell::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    checkArgumentCount( 2, 0 );

    QByteArray domain = args->arg( 0 );
    QByteArray command = args->arg( 1 );

    int fake_argc = 0;
    char **fake_argv = 0;
    SolidShell shell( fake_argc, fake_argv );

    if ( domain == "hardware" )
    {
        if ( command == "list" )
        {
            checkArgumentCount( 2, 3 );
            QByteArray extra( args->count()==3 ? args->arg( 2 ) : "" );
            return shell.hwList( extra=="details" );
        }
        else if ( command == "properties" )
        {
            checkArgumentCount( 3, 3 );
            QString udi( args->arg( 2 ) );
            return shell.hwProperties( udi );
        }
        else if ( command == "query" )
        {
            checkArgumentCount( 3, 4 );

            QString query = args->arg( 2 );
            QString parent;

            if ( args->count() == 4 )
            {
                parent = args->arg( 3 );
            }

            return shell.hwQuery( parent, query );
        }
        else if ( command == "mount" )
        {
            checkArgumentCount( 3, 3 );
            QString udi( args->arg( 2 ) );
            return shell.hwVolumeCall( Mount, udi );
        }
        else if ( command == "unmount" )
        {
            checkArgumentCount( 3, 3 );
            QString udi( args->arg( 2 ) );
            return shell.hwVolumeCall( Unmount, udi );
        }
        else if ( command == "eject" )
        {
            checkArgumentCount( 3, 3 );
            QString udi( args->arg( 2 ) );
            return shell.hwVolumeCall( Eject, udi );
        }
    }


    return false;
}

bool SolidShell::hwList( bool details )
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    KDEHW::DeviceList all = manager.allDevices();

    foreach ( KDEHW::Device device, all )
    {
        cout << "udi = '" << device.udi() << "'" << endl;

        if ( details )
        {
            QMap<QString,QVariant> properties = device.allProperties();
            cout << properties << endl;
        }
    }

    return true;
}

bool SolidShell::hwProperties( const QString &udi )
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();
    KDEHW::Device device = manager.findDevice( udi );

    cout << "udi = '" << device.udi() << "'" << endl;
    QMap<QString,QVariant> properties = device.allProperties();
    cout << properties << endl;

    return true;
}

bool SolidShell::hwQuery( const QString &parentUdi, const QString &query )
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();
    KDEHW::DeviceList devices = manager.findDevicesFromQuery( parentUdi,
                                                              KDEHW::Capability::Unknown,
                                                              query );

    foreach ( KDEHW::Device device, devices )
    {
        cout << "udi = '" << device.udi() << "'" << endl;
    }

    return true;
}

bool SolidShell::hwVolumeCall( SolidShell::VolumeCallType type, const QString &udi )
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();
    KDEHW::Device device = manager.findDevice( udi );

    if ( !device.is<KDEHW::Volume>() )
    {
        cerr << i18n( "Error: %1 doesn't have the capability Volume." ).arg( udi ) << endl;
        return false;
    }

    KJob *job = 0;

    switch( type )
    {
    case Mount:
        job = device.as<KDEHW::Volume>()->mount();
        break;
    case Unmount:
        job = device.as<KDEHW::Volume>()->unmount();
        break;
    case Eject:
        job = device.as<KDEHW::Volume>()->eject();
        break;
    }

    if ( job==0 )
    {
        cerr << i18n( "Error: unsupported operation!" ) << endl;
        return false;
    }

    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotResult( KJob* ) ) );
    
    job->start();
    m_loop.exec();

    if ( m_error )
    {
        cerr << i18n( "Error: %1" ).arg( m_errorString ) << endl;
        return false;
    }
    else
    {
        return true;
    }
}

void SolidShell::slotResult( KJob *job )
{
    m_error = 0;

    if ( job->error() )
    {
        m_error = job->error();
        m_errorString = job->errorString();
    }

    m_loop.exit();
}

#include "main.moc"
