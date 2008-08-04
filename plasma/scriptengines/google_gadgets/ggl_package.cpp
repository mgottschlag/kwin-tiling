/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <ggadget/gadget.h>
#include <ggadget/string_utils.h>
#include "ggl_package.h"

using namespace ggadget;

K_EXPORT_PLASMA_PACKAGESTRUCTURE(googlegadget, GglPackage)

GglPackage::GglPackage(QObject *parent, const QVariantList &args)
  : Plasma::PackageStructure(parent) {
    Q_UNUSED(args);
    setDefaultMimetypes(QStringList() << "application/zip"
                        << "application/x-googlegadget" );
}

GglPackage::~GglPackage() {
}

bool GglPackage::installPackage(const QString &archive_path,
                                const QString &package_root) {
#if 0
  kDebug() << "GglPackage::installPackage archivePath="
           << archive_path
           << "packageRoot=" << package_root;

  PackageInfo pkg(archive_path.toUtf8());
  if (!pkg.SetLocale("1033") && !pkg.SetLocale("en"))
    return false;
  std::string pkg_id = pkg.GetPackageInfo("id");
  std::string pkg_name = pkg.GetPackageInfo("name");
  std::string pkg_icon = pkg.GetPackageInfo("icon");
  std::string pkg_desc = pkg.GetPackageInfo("description");
  kDebug() << "pkg info:\n"
           << pkg_id.c_str() << "\n"
           << pkg_name.c_str() << "\n"
           << pkg_icon.c_str() << "\n"
           << pkg_desc.c_str() << "\n";
  if (pkg_id.empty() || pkg_name.empty()) return false;

  QDir root(package_root);
  const QString dir_name = QString("ggl_%1").arg(pkg_id.c_str());
  if (!root.cd(dir_name) && (!root.mkdir(dir_name) || !root.cd(dir_name))) {
    return false;
  }

  const QString dest_dir =
      QString("%1/%2/").arg(package_root).arg(dir_name);
  QString dest_path = dest_dir + "gadget.gg";
  if (!QFile::copy(archive_path, dest_path))
    return false;

  setPath(dest_dir);

  Plasma::PackageMetadata data;
  // dir_name is used as pluginname
  data.setPluginName(dir_name);
  data.setImplementationApi("googlegadgets");
  data.setType("Service");
  data.setName(pkg_name.c_str());
  data.setDescription(pkg_desc.c_str());

  QString iconfile = "googlegadget";
  if (!pkg_icon.empty()) {
    QString dest_icon = dest_dir + pkg_icon.c_str();
    kDebug() << "Copy Icon:\n"
           << " " << pkg_icon.c_str() << "->" << dest_icon <<"\n";
    if (pkg.ExtractFile(pkg_icon.c_str(), dest_icon.toUtf8()))
      iconfile = QString("%1/%2").arg(dir_name).arg(pkg_icon.c_str());
    kDebug() << "iconfile " << iconfile << "\n";
  }
  Plasma::Package::registerPackage(data, iconfile);
  QFile file(dest_dir + "config");
  file.open(QIODevice::WriteOnly);
  QDataStream out(&file);
  out << dest_path << QString(pkg_id.c_str());
  return true;
#endif
  return false;
}
