/*  This file is part of kdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KLocale>

#include "networkingservice.h"
#include "serviceinterface.h"

QString toString( Solid::Networking::Status st )
{
  QString str;
  switch ( st ) {
    case Solid::Networking::Unknown:
      str = "Unknown";
      break;
    case Solid::Networking::Disconnecting:
      str = "Disconnecting";
      break;
    case Solid::Networking::Unconnected:
      str = "Unconnected";
      break;
    case Solid::Networking::Connecting:
      str = "Connecting";
      break;
    case Solid::Networking::Connected:
      str = "Connected";
      break;
  }
  return str;
}

TestService::TestService() : KMainWindow( 0 ),
    m_service( new OrgKdeSolidNetworkingServiceInterface( "org.kde.kded", "/modules/networkstatus", QDBusConnection::sessionBus(), this ) ),
    m_status ( Solid::Networking::Unconnected ),
    m_nextStatus( Solid::Networking::Unconnected ),
    m_view( new QWidget( this ) )
{
    QDBusConnection::sessionBus().registerService( "org.kde.Solid.Networking.TestService" );

    ui.setupUi( m_view );
    setCentralWidget( m_view );
    connect( ui.changeCombo, SIGNAL( activated( int ) ), SLOT( changeComboActivated( int ) ) );
    connect( ui.changeButton, SIGNAL( clicked() ), SLOT( changeButtonClicked() ) );

    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(const QString&, const QString&, const QString & ) ), SLOT(serviceOwnerChanged(const QString&, const QString&, const QString & ) ) );

    ui.statusLabel->setText( toString( m_status ) );
    QPalette palette;
    palette.setColor( ui.statusLabel->backgroundRole(), toQColor( m_status ) );
    ui.statusLabel->setPalette( palette );
    setCaption( toString( m_status ) );

    registerService();
}

TestService::~TestService()
{
    delete m_service;
}

void TestService::registerService()
{
    m_service->registerNetwork( "test_net", m_status, "org.kde.Solid.Networking.TestService" );
}

void TestService::serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner )
{
    Q_UNUSED( oldOwner );
    if ( !newOwner.isEmpty() && service == "org.kde.kded" ) {
        kDebug() << "KDED restarted, trying to re-register service with it";
        registerService();
    }
}

int TestService::status( const QString & network )
{
    Q_UNUSED( network );
    return (int)m_status;
}

void TestService::changeComboActivated( int index )
{
  switch ( index ) {
    case 0 /*Solid::Networking::Unknown*/:
      m_nextStatus = Solid::Networking::Unknown;
      break;
    case 1 /*Solid::Networking::Unconnected*/:
      m_nextStatus = Solid::Networking::Unconnected;
      break;
    case 2 /*Solid::Networking::Disconnecting*/:
      m_nextStatus = Solid::Networking::Disconnecting;
      break;
    case 3 /*Solid::Networking::Connecting*/:
      m_nextStatus = Solid::Networking::Connecting;
      break;
    case 4 /*Solid::Networking::Connected*/:
      m_nextStatus = Solid::Networking::Connected;
      break;
    default:
      kDebug() << "Unrecognised status!";
      Q_ASSERT( false );
  }
  ui.changeButton->setEnabled( true );
}

void TestService::changeButtonClicked()
{
  ui.changeButton->setEnabled( false );
  m_status = m_nextStatus;
  m_service->setNetworkStatus( "test_net", ( int )m_status );
  ui.statusLabel->setText( toString( m_status ) );
  QPalette palette;
  palette.setColor( ui.statusLabel->backgroundRole(), toQColor( m_status ) );
  ui.statusLabel->setPalette( palette );
  setCaption( toString( m_status ) );
}
#if 0
int TestService::establish( const QString & network )
{
	Q_UNUSED( network );
	m_status = Solid::Networking::Connecting;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = Solid::Networking::Connected;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)Solid::Networking::RequestAccepted;
}

int TestService::shutdown( const QString & network )
{
	Q_UNUSED( network );
	m_status = Solid::Networking::Disconnecting;
	m_service->setNetworkStatus( "test_net", (int)m_status );
	m_nextStatus = Solid::Networking::Unconnected;
	QTimer::singleShot( 5000, this, SLOT( slotStatusChange() ) );
	return (int)Solid::Networking::RequestAccepted;
}

void TestService::simulateFailure()
{
	m_status = Solid::Networking::UnconnectedFailed;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

void TestService::simulateDisconnect()
{
	m_status = Solid::Networking::UnconnectedDisconnected;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}
#endif
void TestService::slotStatusChange()
{
	m_status = m_nextStatus;
	m_service->setNetworkStatus( "test_net", (int)m_status );
}

QColor TestService::toQColor( Solid::Networking::Status st )
{
    QColor col;
    switch ( st ) {
      case Solid::Networking::Unknown:
        col = Qt::darkGray;
        break;
      case Solid::Networking::Disconnecting:
        col = Qt::darkYellow;
        break;
      case Solid::Networking::Unconnected:
        col = Qt::blue;
        break;
      case Solid::Networking::Connecting:
        col = Qt::yellow;
        break;
      case Solid::Networking::Connected:
        col = Qt::green;
        break;
    }
    return col;
}

static const char description[] =
    I18N_NOOP("Test Service for Network Status kded module");

static const char version[] = "v0.1";

int main( int argc, char** argv )
{
    KAboutData about("KNetworkStatusTestService", 0, ki18n("knetworkstatustestservice"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2007 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    TestService * test = new TestService;
    test->show();
    return app.exec();
}

#include "networkingservice.moc"
