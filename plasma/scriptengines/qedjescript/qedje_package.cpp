#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QBuffer>
#include <QDir>
#include <QFile>

#include <plasma/package.h>
#include <plasma/packagestructure.h>
#include <plasma/packagemetadata.h>

#include <qedje_package.h>

K_EXPORT_PLASMA_PACKAGESTRUCTURE(qedjescripts, QEdjePackage)

QEdjePackage::QEdjePackage(QObject *parent, const QVariantList &args)
: Plasma::PackageStructure(parent) {
    Q_UNUSED(args);
    addFileDefinition("edje_file", "file.edj", i18n("Main Edje File"));
    setRequired("edje_file", true);
}

QEdjePackage::~QEdjePackage() {
}


bool QEdjePackage::installPackage(const QString &archive_path,
                                  const QString &package_root) {

  kDebug() << "QEdjePackage::installPackage archivePath="
           << archive_path
           << "packageRoot=" << package_root;

  QString file = archive_path.split("/", QString::SkipEmptyParts).last();
  QString package_name = file.split(".", QString::SkipEmptyParts).first();

  QString dir_name = QString("qedje_%1").arg(package_name);
  QString dest_dir = QString("%1/%2/contents/").arg(package_root).arg(dir_name);

  // Create the directory so we can copy the edje file inside it
  QDir contents(dest_dir);
  if (!contents.exists() && !contents.mkpath(dest_dir))
      return false;

  // Copy the edje file to the package's directory
  if (!QFile::copy(archive_path, QString(dest_dir + "file.edj")))
      return false;

  setPath(dest_dir);

  // Setup the metadata for this package
  Plasma::PackageMetadata data;
  data.setName(package_name);
  data.setType("Service");
  data.setPluginName(dir_name);
  data.setImplementationApi("qedjescript");
  data.setDescription("An Edje Object to be loaded using QEdje");
  Plasma::Package::registerPackage(data, QString(""));

  return true;
}
