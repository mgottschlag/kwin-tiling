#include <iostream>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdehw/devicemanager.h>
#include <kdehw/device.h>

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "solidshell", 0, 0, 0);
    KApplication app(false);

    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    KDEHW::DeviceList all = manager.allDevices();

    foreach ( KDEHW::Device device, all )
    {
        std::cout << "udi = " << device.udi().toLatin1().constData() << std::endl;

        QMap<QString,QVariant> properties = device.allProperties();

        foreach( QString key, properties.keys() )
        {
            std::cout << "    " << key.toLatin1().constData() << " = "
                      << "'" << properties[key].toString().toLatin1().constData()
                      << "'" << std::endl;
        }
    }

    return 0;
}
