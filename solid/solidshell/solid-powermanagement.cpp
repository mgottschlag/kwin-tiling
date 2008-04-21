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

#include "solid-powermanagement.h"


#include <QString>
#include <QStringList>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QTimer>

#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <k3socketaddress.h>
#include <kdebug.h>

#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/storageaccess.h>
#include <solid/opticaldrive.h>

#include <solid/control/powermanager.h>

#include <kjob.h>


#include <iostream>
using namespace std;

static const char appName[] = "solid-powermanagement";
static const char programName[] = I18N_NOOP("solid-powermanagement");

static const char description[] = I18N_NOOP("KDE tool for querying and controlling your power management options from the command line");

static const char version[] = "0.1";

std::ostream &operator<<(std::ostream &out, const QString &msg)
{
    return (out << msg.toLocal8Bit().constData());
}

std::ostream &operator<<(std::ostream &out, const QVariant &value)
{
    switch (value.type())
    {
    case QVariant::StringList:
    {
        out << "{";

        QStringList list = value.toStringList();

        QStringList::ConstIterator it = list.begin();
        QStringList::ConstIterator end = list.end();

        for (; it!=end; ++it)
        {
            out << "'" << *it << "'";

            if (it+1!=end)
            {
                out << ", ";
            }
        }

        out << "}  (string list)";
        break;
    }
    case QVariant::Bool:
        out << (value.toBool()?"true":"false") << "  (bool)";
        break;
    case QVariant::Int:
        out << value.toString()
            << "  (0x" << QString::number(value.toInt(), 16) << ")  (int)";
        break;
    default:
        out << "'" << value.toString() << "'  (string)";
        break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const Solid::Device &device)
{
    out << "  parent = " << QVariant(device.parentUdi()) << endl;
    out << "  vendor = " << QVariant(device.vendor()) << endl;
    out << "  product = " << QVariant(device.product()) << endl;

    int index = Solid::DeviceInterface::staticMetaObject.indexOfEnumerator("Type");
    QMetaEnum typeEnum = Solid::DeviceInterface::staticMetaObject.enumerator(index);

    for (int i=0; i<typeEnum.keyCount(); i++)
    {
        Solid::DeviceInterface::Type type = (Solid::DeviceInterface::Type)typeEnum.value(i);
        const Solid::DeviceInterface *interface = device.asDeviceInterface(type);

        if (interface)
        {
            const QMetaObject *meta = interface->metaObject();

            for (int i=meta->propertyOffset(); i<meta->propertyCount(); i++)
            {
                QMetaProperty property = meta->property(i);
                out << "  " << QString(meta->className()).mid(7) << "." << property.name()
                    << " = ";

                QVariant value = property.read(interface);

                if (property.isEnumType()) {
                    QMetaEnum metaEnum = property.enumerator();
                    out << "'" << metaEnum.valueToKeys(value.toInt()).constData() << "'"
                        << "  (0x" << QString::number(value.toInt(), 16) << ")  ";
                    if (metaEnum.isFlag()) {
                        out << "(flag)";
                    } else {
                        out << "(enum)";
                    }
                    out << endl;
                } else {
                    out << value << endl;
                }
            }
        }
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, const QMap<QString,QVariant> &properties)
{
    foreach (QString key, properties.keys())
    {
        out << "  " << key << " = " << properties[key] << endl;
    }

    return out;
}

void checkArgumentCount(int min, int max)
{
    int count = KCmdLineArgs::parsedArgs()->count();

    if (count < min)
    {
        cerr << i18n("Syntax Error: Not enough arguments") << endl;
        ::exit(1);
    }

    if ((max > 0) && (count > max))
    {
        cerr << i18n("Syntax Error: Too many arguments") << endl;
        ::exit(1);
    }
}

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, appName, 0, ki18n(programName), version, ki18n(description), false);


  KCmdLineOptions options;

  options.add("commands", ki18n("Show available commands"));

  options.add("+command", ki18n("Command (see --commands)"));

  options.add("+[arg(s)]", ki18n("Arguments for command"));

  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KComponentData componentData(appName);

  if (args->isSet("commands"))
  {
      KCmdLineArgs::enable_i18n();

      cout << endl << i18n("Syntax:") << endl << endl;

      cout << "  solid-powermanagement query (suspend|scheme|cpufreq)" << endl;
      cout << i18n("             # List a particular set of information regarding power management.\n"
                    "             # - If the 'suspend' option is specified, give the list of suspend\n"
                    "             # method supported by the system\n"
                    "             # - If the 'scheme' option is specified, give the list of\n"
                    "             # supported power management schemes by this system\n"
                    "             # - If the 'cpufreq' option is specified, give the list of\n"
                    "             # supported CPU frequency policy\n") << endl;

      cout << "  solid-powermanagement set (scheme|cpufreq) 'value'" << endl;
      cout << i18n("             # Set power management options of the system.\n"
                    "             # - If the 'scheme' option is specified, the power management\n"
                    "             # scheme set corresponds to 'value'\n"
                    "             # - If the 'cpufreq' option is specified, the CPU frequency policy\n"
                    "             # set corresponds to 'value'\n") << endl;

      cout << "  solid-powermanagement suspend 'method'" << endl;
      cout << i18n("             # Suspend the computer using the given 'method'.\n") << endl;

      cout << "  solid-powermanagement brightness (set|get) 'value'" << endl;
      cout << i18n("             # Set and get brightness options of the system.\n"
                    "             # - If the 'set' option is specified, the brightness is\n"
                    "             # set to 'value' (as a percentage)\n"
                    "             # - If the 'get' option is specified, the current brightness\n"
                    "             # is returned (as a percentage)'\n") << endl;

      cout << endl;

      return 0;
  }

  return SolidPowermanagement::doIt() ? 0 : 1;
}

bool SolidPowermanagement::doIt()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    checkArgumentCount(1, 0);

    QString command(args->arg(0));

    int fake_argc = 0;
    char **fake_argv = 0;
    SolidPowermanagement shell(fake_argc, fake_argv);

    if (command == "suspend")
    {
        checkArgumentCount(2, 2);
        QString method(args->arg(1));

        return shell.powerSuspend(method);
    }
    else if (command == "query")
    {
        checkArgumentCount(2, 2);
        QString type(args->arg(1));

        if (type == "suspend")
        {
            return shell.powerQuerySuspendMethods();
        }
        else if (type == "scheme")
        {
            return shell.powerQuerySchemes();
        }
        else if (type == "cpufreq")
        {
            return shell.powerQueryCpuPolicies();
        }
        else
        {
            cerr << i18n("Syntax Error: Unknown option '%1'" , type) << endl;
        }
    }
    else if (command == "set")
    {
        checkArgumentCount(3, 3);
        QString type(args->arg(1));
        QString value(args->arg(2));
    
        if (type == "scheme")
        {
            return shell.powerChangeScheme(value);
        }
        else if (type == "cpufreq")
        {
            return shell.powerChangeCpuPolicy(value);
        }
        else
        {
            cerr << i18n("Syntax Error: Unknown option '%1'" , type) << endl;
        }
    }
    else if (command == "brightness")
    {
        QString request(args->arg(1));
        if (request == "get")
        {
            cout << endl << "Brightness is " << shell.powerGetBrightness() << "%" << endl;
        }
        else if (request == "set")
        {
            return shell.powerSetBrightness(args->arg(2).toInt());
        }
    }
    else
    {
        cerr << i18n("Syntax Error: Unknown command '%1'" , command) << endl;
    }

    return false;
}

bool SolidPowermanagement::powerQuerySuspendMethods()
{
    Solid::Control::PowerManager::SuspendMethods methods = Solid::Control::PowerManager::supportedSuspendMethods();

    if (methods  & Solid::Control::PowerManager::ToDisk)
    {
        cout << "to_disk" << endl;
    }

    if (methods  & Solid::Control::PowerManager::ToRam)
    {
        cout << "to_ram" << endl;
    }

    if (methods  & Solid::Control::PowerManager::Standby)
    {
        cout << "standby" << endl;
    }

    return true;
}

bool SolidPowermanagement::powerSuspend(const QString &strMethod)
{
    Solid::Control::PowerManager::SuspendMethods supported
        = Solid::Control::PowerManager::supportedSuspendMethods();

    Solid::Control::PowerManager::SuspendMethod method = Solid::Control::PowerManager::UnknownSuspendMethod;

    if (strMethod == "to_disk" && (supported  & Solid::Control::PowerManager::ToDisk))
    {
        method = Solid::Control::PowerManager::ToDisk;
    }
    else if (strMethod == "to_ram" && (supported  & Solid::Control::PowerManager::ToRam))
    {
        method = Solid::Control::PowerManager::ToRam;
    }
    else if (strMethod == "standby" && (supported  & Solid::Control::PowerManager::Standby))
    {
        method = Solid::Control::PowerManager::Standby;
    }
    else
    {
        cerr << i18n("Unsupported suspend method: %1" , strMethod) << endl;
        return false;
    }

    KJob *job = Solid::Control::PowerManager::suspend(method);

    if (job==0)
    {
        cerr << i18n("Error: unsupported operation!") << endl;
        return false;
    }

    connectJob(job);

    job->start();
    m_loop.exec();

    if (m_error)
    {
        cerr << i18n("Error: %1" , m_errorString) << endl;
        return false;
    }
    else
    {
        return true;
    }
}

bool SolidPowermanagement::powerQuerySchemes()
{
    QString current = Solid::Control::PowerManager::scheme();
    QStringList schemes = Solid::Control::PowerManager::supportedSchemes();

    foreach (QString scheme, schemes)
    {
        cout << scheme << " (" << Solid::Control::PowerManager::schemeDescription(scheme) << ")";

        if (scheme==current)
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

bool SolidPowermanagement::powerChangeScheme(const QString &schemeName)
{
    QStringList supported = Solid::Control::PowerManager::supportedSchemes();

    if (!supported.contains(schemeName))
    {
        cerr << i18n("Unsupported scheme: %1" , schemeName) << endl;
        return false;
    }

    return Solid::Control::PowerManager::setScheme(schemeName);
}

bool SolidPowermanagement::powerQueryCpuPolicies()
{
    Solid::Control::PowerManager::CpuFreqPolicy current = Solid::Control::PowerManager::cpuFreqPolicy();
    Solid::Control::PowerManager::CpuFreqPolicies policies = Solid::Control::PowerManager::supportedCpuFreqPolicies();

    QList<Solid::Control::PowerManager::CpuFreqPolicy> all_policies;
    all_policies << Solid::Control::PowerManager::OnDemand
                 << Solid::Control::PowerManager::Userspace
                 << Solid::Control::PowerManager::Powersave
                 << Solid::Control::PowerManager::Performance
                 << Solid::Control::PowerManager::Conservative;

    foreach (Solid::Control::PowerManager::CpuFreqPolicy policy, all_policies)
    {
        if (policies  & policy)
        {
            switch (policy)
            {
            case Solid::Control::PowerManager::OnDemand:
                cout << "ondemand";
                break;
            case Solid::Control::PowerManager::Userspace:
                cout << "userspace";
                break;
            case Solid::Control::PowerManager::Powersave:
                cout << "powersave";
                break;
            case Solid::Control::PowerManager::Performance:
                cout << "performance";
                break;
            case Solid::Control::PowerManager::Conservative:
                cout << "conservative";
                break;
            case Solid::Control::PowerManager::UnknownCpuFreqPolicy:
                break;
            }

            if (policy==current)
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

bool SolidPowermanagement::powerChangeCpuPolicy(const QString &policyName)
{
    Solid::Control::PowerManager::CpuFreqPolicies supported
        = Solid::Control::PowerManager::supportedCpuFreqPolicies();

    Solid::Control::PowerManager::CpuFreqPolicy policy = Solid::Control::PowerManager::UnknownCpuFreqPolicy;

    if (policyName == "ondemand" && (supported  & Solid::Control::PowerManager::OnDemand))
    {
        policy = Solid::Control::PowerManager::OnDemand;
    }
    else if (policyName == "userspace" && (supported  & Solid::Control::PowerManager::Userspace))
    {
        policy = Solid::Control::PowerManager::Userspace;
    }
    else if (policyName == "performance" && (supported  & Solid::Control::PowerManager::Performance))
    {
        policy = Solid::Control::PowerManager::Performance;
    }
    else if (policyName == "powersave" && (supported  & Solid::Control::PowerManager::Powersave))
    {
        policy = Solid::Control::PowerManager::Powersave;
    }
    else if (policyName == "conservative" && (supported  & Solid::Control::PowerManager::Conservative))
    {
        policy = Solid::Control::PowerManager::Conservative;
    }
    else
    {
        cerr << i18n("Unsupported cpufreq policy: %1" , policyName) << endl;
        return false;
    }

    return Solid::Control::PowerManager::setCpuFreqPolicy(policy);
}

int SolidPowermanagement::powerGetBrightness()
{
    return Solid::Control::PowerManager::brightness();
}

bool SolidPowermanagement::powerSetBrightness(int brightness)
{
    cout << "Setting brightness to " << brightness << "%" << endl;
    return Solid::Control::PowerManager::setBrightness(brightness);
}


void SolidPowermanagement::connectJob(KJob *job)
{
    connect(job, SIGNAL(result(KJob *)),
             this, SLOT(slotResult(KJob *)));
    connect(job, SIGNAL(percent(KJob *, unsigned long)),
             this, SLOT(slotPercent(KJob *, unsigned long)));
    connect(job, SIGNAL(infoMessage(KJob *, const QString &, const QString &)),
             this, SLOT(slotInfoMessage(KJob *, const QString &)));
}

void SolidPowermanagement::slotPercent(KJob */*job */, unsigned long percent)
{
    cout << i18n("Progress: %1%" , percent) << endl;
}

void SolidPowermanagement::slotInfoMessage(KJob */*job */, const QString &message)
{
    cout << i18n("Info: %1" , message) << endl;
}

void SolidPowermanagement::slotResult(KJob *job)
{
    m_error = 0;

    if (job->error())
    {
        m_error = job->error();
        m_errorString = job->errorString();
    }

    m_loop.exit();
}

void SolidPowermanagement::slotStorageResult(Solid::ErrorType error, const QVariant &errorData)
{
    if (error) {
        m_error = 1;
        m_errorString = errorData.toString();
    }
    m_loop.exit();
}

#include "solid-powermanagement.moc"
