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
#include <QtGui/QFontDatabase>
#include <QtGui/QMessageBox>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <ggadget/gadget.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/host_interface.h>
#include <ggadget/string_utils.h>
#include <ggadget/gadget_manager_interface.h>
#include <ggadget/system_utils.h>
#include <ggadget/view.h>
#include <ggadget/messages.h>
#include <ggadget/permissions.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/qt/utilities.h>
#include "ggl_extensions.h"
#include "ggl_package.h"

using namespace ggadget;

K_EXPORT_PLASMA_PACKAGESTRUCTURE(googlegadget, GglPackage)

class GadgetBrowserHost : public ggadget::HostInterface {
 public:
  GadgetBrowserHost() : gadget_manager_(GetGadgetManager()) {
    kDebug() << "Create GadgetBrowserHost:" << this;
    connection_ = GetGadgetManager()->ConnectOnNewGadgetInstance(
        NewSlot(this, &GadgetBrowserHost::NewGadgetInstanceCallback));
  }

  ~GadgetBrowserHost() {
    kDebug() << "Destroy GadgetBrowserHost:" << this;
    connection_->Disconnect();
  }

  bool InstallPlasmaApplet(int id) {
    std::string author, download_url, title, description;
    if (!gadget_manager_->GetGadgetInstanceInfo(id, "", &author, &download_url,
                                                &title, &description))
      return false;
    std::string path = gadget_manager_->GetGadgetInstancePath(id).c_str();
    std::string options = gadget_manager_->GetGadgetInstanceOptionsName(id);
    QString pkg_name = QString("ggl_%1").arg(id);

    // Create package
    QString plasmods_dir =
        KStandardDirs::locateLocal("data", "plasma/plasmoids/");
    QDir root(plasmods_dir);
    if (!root.cd(pkg_name) &&
        (!root.mkpath(pkg_name) || !root.cd(pkg_name))) {
      LOGE("Failed to create package %s",
           (root.path() + "/" + pkg_name).toUtf8().data());
      return false;
    }
    {
      QFile file(root.path() + "/config.txt");
      file.open(QIODevice::WriteOnly);
      QTextStream out(&file);
      out << QString::fromUtf8(path.c_str())  << "\n";
      out << QString::fromUtf8(options.c_str()) << "\n";
    }

    // Register package
    Plasma::PackageMetadata data;
    data.setPluginName(pkg_name);
    data.setType("Service");
    data.setImplementationApi("googlegadgets");
    data.setName(QString::fromUtf8(title.c_str()));
    data.setDescription(QString::fromUtf8(description.c_str()));

    Plasma::Package::registerPackage(data, "google-gadgets");
    return true;
  }

  bool NewGadgetInstanceCallback(int id) {
    if (ggadget::qt::ConfirmGadget(gadget_manager_, id)) {
        return InstallPlasmaApplet(id);
    } else {
      QMessageBox::information(
          NULL,
          QString::fromUtf8(GM_("GOOGLE_GADGETS")),
          QString::fromUtf8(
              StringPrintf(
                  GM_("GADGET_LOAD_FAILURE"),
                  gadget_manager_->GetGadgetInstancePath(id).c_str()).c_str()));
      return false;
    }
  }

  virtual ViewHostInterface *NewViewHost(Gadget *gadget,
                                         ViewHostInterface::Type type) {
    return new qt::QtViewHost(type, 1.0, false, true, false,
                              0, NULL);
  }
  virtual void RemoveGadget(Gadget *gadget, bool save_data) { }
  virtual bool LoadFont(const char *filename) {
    if (QFontDatabase::addApplicationFont(filename) != -1)
      return true;
    else
      return false;
  }
  virtual void Run() {}
  virtual void ShowGadgetAboutDialog(Gadget*) { }
  virtual void ShowGadgetDebugConsole(Gadget*) {}
  virtual int GetDefaultFontSize() { return ggadget::kDefaultFontSize; }
  virtual bool OpenURL(const Gadget *, const char *) { return false; }

  GadgetManagerInterface *gadget_manager_;
  Connection *connection_;
};

GglPackage::GglPackage(QObject *parent, const QVariantList &args)
  : Plasma::PackageStructure(parent), host_(NULL) {
  Q_UNUSED(args);
  std::string profile_dir =
      ggadget::BuildFilePath(ggadget::GetHomeDirectory().c_str(),
                             ".google/gadgets-plasma", NULL);

  std::string error;
  if (!ggadget::qt::InitGGL(NULL, "ggl-plasma", profile_dir.c_str(),
                            kGlobalExtensions, 0, false, &error)) {
    kError() << "Failed to init GGL system:"
             << QString::fromUtf8(error.c_str());
    return;
  }
  host_ = new GadgetBrowserHost();

  setDefaultMimetypes(QStringList() << "application/zip"
                      << "application/x-googlegadget" );
}

GglPackage::~GglPackage() {
  delete host_;
}

bool GglPackage::installPackage(const QString &archive_path,
                                const QString &package_root) {
  if (host_->gadget_manager_->NewGadgetInstanceFromFile(
      archive_path.toUtf8().data()) == -1)
    return false;
  return true;
}

void GglPackage::createNewWidgetBrowser(QWidget *parent) {
  if (!host_) {
    emit newWidgetBrowserFinished();
    return;
  }
  GetGadgetManager()->ShowGadgetBrowserDialog(host_);
}
