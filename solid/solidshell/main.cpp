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


#include <QString>
#include <QStringList>
#include <QMetaProperty>

#include <kinstance.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/volume.h>

#include <solid/powermanager.h>

#include <kjob.h>


#include <iostream>
using namespace std;

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

std::ostream &operator<<( std::ostream &out, const QVariant &value )
{
    switch ( value.type() )
    {
    case QVariant::StringList:
    {
        out << "{";

        QStringList list = value.toStringList();

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

        out << "}  (string list)";
        break;
    }
    case QVariant::Bool:
        out << ( value.toBool()?"true":"false" ) << "  (bool)";
        break;
    case QVariant::Int:
        out << value.toString()
            << "  (0x" << QString::number( value.toInt(), 16 ) << ")  (int)";
        break;
    default:
        out << "'" << value.toString() << "'  (string)";
        break;
    }

    return out;
}

std::ostream &operator<<( std::ostream &out, const Solid::Device &device )
{
    out << "  parent = " << QVariant( device.parentUdi() ) << endl;
    out << "  vendor = " << QVariant( device.vendor() ) << endl;
    out << "  product = " << QVariant( device.product() ) << endl;

    QList<Solid::Capability::Type> caps;
    caps << Solid::Capability::Processor
         << Solid::Capability::Block
         << Solid::Capability::Storage
         << Solid::Capability::Cdrom
         << Solid::Capability::Volume
         << Solid::Capability::OpticalDisc
         << Solid::Capability::Camera
         << Solid::Capability::PortableMediaPlayer
         << Solid::Capability::NetworkHw
         << Solid::Capability::AcAdapter
         << Solid::Capability::Battery
         << Solid::Capability::Button
         << Solid::Capability::Display
         << Solid::Capability::AudioHw;

    foreach ( Solid::Capability::Type cap, caps )
    {
        const Solid::Capability *capability = device.asCapability( cap );

        if ( capability )
        {
            const QMetaObject *meta = capability->metaObject();

            for ( int i=meta->propertyOffset(); i<meta->propertyCount(); i++ )
            {
                QMetaProperty property = meta->property( i );
                out << "  " << QString( meta->className() ).mid( 7 ) << "." << property.name()
                    << " = " << property.read( capability ) << endl;
            }
        }
    }

    return out;
}

std::ostream &operator<<( std::ostream &out, const QMap<QString,QVariant> &properties )
{
    foreach ( QString key, properties.keys() )
    {
        out << "  " << key << " = " << properties[key] << endl;
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

      cout << "  solidshell hardware list [details|nonportableinfo]" << endl;
      cout << i18n( "             # List the hardware available in the system.\n"
                    "             # - If the 'nonportableinfo' option is specified, the device\n"
                    "             # properties are listed (be careful, in this case property names\n"
                    "             # are backend dependent),\n"
                    "             # - If the 'details' option is specified, the device capabilities\n"
                    "             # and the corresponding properties are listed in a platform\n"
                    "             # neutral fashion,\n"
                    "             # - Otherwise only device UDIs are listed.\n" ) << endl;

      cout << "  solidshell hardware details 'udi'" << endl;
      cout << i18n( "             # Display all the capabilities and properties of the device\n"
                    "             # corresponding to 'udi' in a platform neutral fashion.\n" ) << endl;

      cout << "  solidshell hardware nonportableinfo 'udi'" << endl;
      cout << i18n( "             # Display all the properties of the device corresponding to 'udi'\n"
                    "             # (be careful, in this case property names are backend dependent).\n" ) << endl;

      cout << "  solidshell hardware query 'predicate' ['parentUdi']" << endl;
      cout << i18n( "             # List the UDI of devices corresponding to 'predicate'.\n"
                    "             # - If 'parentUdi' is specified, the search is restricted to the\n"
                    "             # branch of the corresponding device,\n"
                    "             # - Otherwise the search is done on all the devices.\n" ) << endl;

      cout << "  solidshell hardware mount 'udi'" << endl;
      cout << i18n( "             # If applicable, mount the device corresponding to 'udi'.\n" ) << endl;

      cout << "  solidshell hardware unmount 'udi'" << endl;
      cout << i18n( "             # If applicable, unmount the device corresponding to 'udi'.\n" ) << endl;

      cout << "  solidshell hardware eject 'udi'" << endl;
      cout << i18n( "             # If applicable, eject the device corresponding to 'udi'.\n" ) << endl;


      cout << endl;


      cout << "  solidshell power query (suspend|scheme|cpufreq)" << endl;
      cout << i18n( "             # List a particular set of information regarding power management.\n"
                    "             # - If the 'suspend' option is specified, give the list of suspend\n"
                    "             # method supported by the system\n"
                    "             # - If the 'scheme' option is specified, give the list of\n"
                    "             # supported power management schemes by this system\n"
                    "             # - If the 'cpufreq' option is specified, give the list of\n"
                    "             # supported CPU frequency policy\n" ) << endl;

      cout << "  solidshell power set (scheme|cpufreq) 'value'" << endl;
      cout << i18n( "             # Set power management options of the system.\n"
                    "             # - If the 'scheme' option is specified, the power management\n"
                    "             # scheme set corresponds to 'value'\n"
                    "             # - If the 'cpufreq' option is specified, the CPU frequency policy\n"
                    "             # set corresponds to 'value'\n" ) << endl;

      cout << "  solidshell power suspend 'method'" << endl;
      cout << i18n( "             # Suspend the computer using the given 'method'.\n" ) << endl;

      return 0;
  }

  return SolidShell::doIt() ? 0 : 1;
}

bool SolidShell::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    checkArgumentCount( 2, 0 );

    QString domain( args->arg( 0 ) );
    QString command( args->arg( 1 ) );

    int fake_argc = 0;
    char **fake_argv = 0;
    SolidShell shell( fake_argc, fake_argv );

    if ( domain == "hardware" )
    {
        if ( command == "list" )
        {
            checkArgumentCount( 2, 3 );
            QByteArray extra( args->count()==3 ? args->arg( 2 ) : "" );
            return shell.hwList( extra=="details", extra=="nonportableinfo" );
        }
        else if ( command == "details" )
        {
            checkArgumentCount( 3, 3 );
            QString udi( args->arg( 2 ) );
            return shell.hwCapabilities( udi );
        }
        else if ( command == "nonportableinfo" )
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
        else
        {
            cerr << i18n( "Syntax Error: Unknown command '%1'" ,command ) << endl;
        }
    }
    else if ( domain == "power" )
    {
        if ( command == "suspend" )
        {
            checkArgumentCount( 3, 3 );
            QString method( args->arg( 2 ) );

            return shell.powerSuspend( method );
        }
        else if ( command == "query" )
        {
            checkArgumentCount( 3, 3 );
            QString type( args->arg( 2 ) );

            if ( type == "suspend" )
            {
                return shell.powerQuerySuspendMethods();
            }
            else if ( type == "scheme" )
            {
                return shell.powerQuerySchemes();
            }
            else if ( type == "cpufreq" )
            {
                return shell.powerQueryCpuPolicies();
            }
            else
            {
                cerr << i18n( "Syntax Error: Unknown option '%1'" , type ) << endl;
            }
        }
        else if ( command == "set" )
        {
            checkArgumentCount( 4, 4 );
            QString type( args->arg( 2 ) );
            QString value( args->arg( 3 ) );

            if ( type == "scheme" )
            {
                return shell.powerChangeScheme( value );
            }
            else if ( type == "cpufreq" )
            {
                return shell.powerChangeCpuPolicy( value );
            }
            else
            {
                cerr << i18n( "Syntax Error: Unknown option '%1'" , type ) << endl;
            }
        }
        else
        {
            cerr << i18n( "Syntax Error: Unknown command '%1'" , command ) << endl;
        }
    }
    else
    {
        cerr << i18n( "Syntax Error: Unknown command group '%1'" , domain ) << endl;
    }

    return false;
}

bool SolidShell::hwList( bool capabilities, bool system )
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    const Solid::DeviceList all = manager.allDevices();

    foreach ( const Solid::Device device, all )
    {
        cout << "udi = '" << device.udi() << "'" << endl;

        if ( capabilities )
        {
            cout << device << endl;
        }
        else if ( system )
        {
            QMap<QString,QVariant> properties = device.allProperties();
            cout << properties << endl;
        }
    }

    return true;
}

bool SolidShell::hwCapabilities( const QString &udi )
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    const Solid::Device device = manager.findDevice( udi );

    cout << "udi = '" << device.udi() << "'" << endl;
    cout << device << endl;

    return true;
}

bool SolidShell::hwProperties( const QString &udi )
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    const Solid::Device device = manager.findDevice( udi );

    cout << "udi = '" << device.udi() << "'" << endl;
    QMap<QString,QVariant> properties = device.allProperties();
    cout << properties << endl;

    return true;
}

bool SolidShell::hwQuery( const QString &parentUdi, const QString &query )
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    const Solid::DeviceList devices = manager.findDevicesFromQuery( Solid::Capability::Unknown,
                                                                    query, parentUdi );

    foreach ( const Solid::Device device, devices )
    {
        cout << "udi = '" << device.udi() << "'" << endl;
    }

    return true;
}

bool SolidShell::hwVolumeCall( SolidShell::VolumeCallType type, const QString &udi )
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    Solid::Device device = manager.findDevice( udi );

    if ( !device.is<Solid::Volume>() )
    {
        cerr << i18n( "Error: %1 does not have the capability Volume." , udi ) << endl;
        return false;
    }

    KJob *job = 0;

    switch( type )
    {
    case Mount:
        job = device.as<Solid::Volume>()->mount();
        break;
    case Unmount:
        job = device.as<Solid::Volume>()->unmount();
        break;
    case Eject:
        job = device.as<Solid::Volume>()->eject();
        break;
    }

    if ( job==0 )
    {
        cerr << i18n( "Error: unsupported operation!" ) << endl;
        return false;
    }

    connectJob( job );

    job->start();
    m_loop.exec();

    if ( m_error )
    {
        cerr << i18n( "Error: %1", m_errorString ) << endl;
        return false;
    }
    else
    {
        return true;
    }
}

bool SolidShell::powerQuerySuspendMethods()
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    Solid::PowerManager::SuspendMethods methods = manager.supportedSuspendMethods();

    if ( methods & Solid::PowerManager::ToDisk )
    {
        cout << "to_disk" << endl;
    }

    if ( methods & Solid::PowerManager::ToRam )
    {
        cout << "to_ram" << endl;
    }

    if ( methods & Solid::PowerManager::Standby )
    {
        cout << "standby" << endl;
    }

    return true;
}

bool SolidShell::powerSuspend( const QString &strMethod )
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    Solid::PowerManager::SuspendMethods supported = manager.supportedSuspendMethods();

    Solid::PowerManager::SuspendMethod method = Solid::PowerManager::UnknownSuspendMethod;

    if ( strMethod == "to_disk" && (supported & Solid::PowerManager::ToDisk) )
    {
        method = Solid::PowerManager::ToDisk;
    }
    else if ( strMethod == "to_ram" && (supported & Solid::PowerManager::ToRam) )
    {
        method = Solid::PowerManager::ToRam;
    }
    else if ( strMethod == "standby" && (supported & Solid::PowerManager::Standby) )
    {
        method = Solid::PowerManager::Standby;
    }
    else
    {
        cerr << i18n( "Unsupported suspend method: %1" , strMethod ) << endl;
        return false;
    }

    KJob *job = manager.suspend( method );

    if ( job==0 )
    {
        cerr << i18n( "Error: unsupported operation!" ) << endl;
        return false;
    }

    connectJob( job );

    job->start();
    m_loop.exec();

    if ( m_error )
    {
        cerr << i18n( "Error: %1" , m_errorString ) << endl;
        return false;
    }
    else
    {
        return true;
    }
}

bool SolidShell::powerQuerySchemes()
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    QString current = manager.scheme();
    QStringList schemes = manager.supportedSchemes();

    foreach ( QString scheme, schemes )
    {
        cout << scheme << " (" << manager.schemeDescription( scheme ) << ")";

        if ( scheme==current )
        {
            cout << " [*]" << endl;
        }
        else
        {
            cout << endl;
        }
    }

    return true;
}

bool SolidShell::powerChangeScheme( const QString &schemeName )
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    QStringList supported = manager.supportedSchemes();

    if ( !supported.contains( schemeName ) )
    {
        cerr << i18n( "Unsupported scheme: %1" , schemeName ) << endl;
        return false;
    }

    return manager.setScheme( schemeName );
}

bool SolidShell::powerQueryCpuPolicies()
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    Solid::PowerManager::CpuFreqPolicy current = manager.cpuFreqPolicy();
    Solid::PowerManager::CpuFreqPolicies policies = manager.supportedCpuFreqPolicies();

    QList<Solid::PowerManager::CpuFreqPolicy> all_policies;
    all_policies << Solid::PowerManager::OnDemand
                 << Solid::PowerManager::Userspace
                 << Solid::PowerManager::Powersave
                 << Solid::PowerManager::Performance;

    foreach ( Solid::PowerManager::CpuFreqPolicy policy, all_policies )
    {
        if ( policies & policy )
        {
            switch ( policy )
            {
            case Solid::PowerManager::OnDemand:
                cout << "ondemand";
                break;
            case Solid::PowerManager::Userspace:
                cout << "userspace";
                break;
            case Solid::PowerManager::Powersave:
                cout << "powersave";
                break;
            case Solid::PowerManager::Performance:
                cout << "performance";
                break;
            case Solid::PowerManager::UnknownCpuFreqPolicy:
                break;
            }

            if ( policy==current )
            {
                cout << " [*]" << endl;
            }
            else
            {
                cout << endl;
            }
        }
    }

    return true;
}

bool SolidShell::powerChangeCpuPolicy( const QString &policyName )
{
    Solid::PowerManager &manager = Solid::PowerManager::self();

    Solid::PowerManager::CpuFreqPolicies supported = manager.supportedCpuFreqPolicies();

    Solid::PowerManager::CpuFreqPolicy policy = Solid::PowerManager::UnknownCpuFreqPolicy;

    if ( policyName == "ondemand" && (supported & Solid::PowerManager::OnDemand) )
    {
        policy = Solid::PowerManager::OnDemand;
    }
    else if ( policyName == "userspace" && (supported & Solid::PowerManager::Userspace) )
    {
        policy = Solid::PowerManager::Userspace;
    }
    else if ( policyName == "performance" && (supported & Solid::PowerManager::Performance) )
    {
        policy = Solid::PowerManager::Performance;
    }
    else if ( policyName == "powersave" && (supported & Solid::PowerManager::Powersave) )
    {
        policy = Solid::PowerManager::Powersave;
    }
    else
    {
        cerr << i18n( "Unsupported cpufreq policy: %1" , policyName ) << endl;
        return false;
    }

    return manager.setCpuFreqPolicy( policy );
}

void SolidShell::connectJob( KJob *job )
{
    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotResult( KJob* ) ) );
    connect( job, SIGNAL( percent( KJob*, unsigned long ) ),
             this, SLOT( slotPercent( KJob*, unsigned long ) ) );
    connect( job, SIGNAL( infoMessage( KJob*, const QString&, const QString& ) ),
             this, SLOT( slotInfoMessage( KJob*, const QString& ) ) );
}

void SolidShell::slotPercent( KJob */*job*/, unsigned long percent )
{
    cout << i18n( "Progress: %1%" , percent ) << endl;
}

void SolidShell::slotInfoMessage( KJob */*job*/, const QString &message )
{
    cout << i18n( "Info: %1" , message ) << endl;
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
