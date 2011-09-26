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
#include "ggl_package.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QFontDatabase>
#include <QtGui/QMessageBox>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ggadget/gadget.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/host_interface.h>
#include <ggadget/string_utils.h>
#include <ggadget/gadget_manager_interface.h>
#include <ggadget/file_manager_interface.h>
#include <ggadget/system_utils.h>
#include <ggadget/scoped_ptr.h>
#include <ggadget/view.h>
#include <ggadget/messages.h>
#include <ggadget/permissions.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/qt/utilities.h>
#include "ggl_extensions.h"

using namespace ggadget;

K_EXPORT_PLASMA_PACKAGESTRUCTURE(googlegadget, GglPackage)

class GadgetBrowserViewHost : public qt::QtViewHost {
 public:
  GadgetBrowserViewHost(GglPackage *package, Type type)
      : QtViewHost(type, 1.0, FLAG_RECORD_STATES, 0, NULL),
        package_(package) {}

  virtual void CloseView() {
    package_->gadgetBrowserClosed();
  }
  GglPackage *package_;
};

class GadgetBrowserHost : public ggadget::HostInterface {
 public:
  GadgetBrowserHost(GglPackage *package)
      : gadget_manager_(NULL),
        package_(package),
        connection_(NULL) {
    kDebug() << "Create GadgetBrowserHost:" << this;
    std::string profile_dir =
        ggadget::BuildFilePath(ggadget::GetHomeDirectory().c_str(),
                               ".google/gadgets-plasma", NULL);

    QString error;
    if (!ggadget::qt::InitGGL(NULL, "ggl-plasma", profile_dir.c_str(),
                              kGlobalExtensions, 0,
                              ggadget::qt::GGL_INIT_FLAG_COLLECTOR, &error)) {
      kError() << "Failed to init GGL system:" << error;
      return;
    }
    gadget_manager_ = GetGadgetManager();
    connection_ = gadget_manager_->ConnectOnNewGadgetInstance(
        NewSlot(this, &GadgetBrowserHost::newGadgetInstanceCallback));
  }

  ~GadgetBrowserHost() {
    kDebug() << "Destroy GadgetBrowserHost:" << this;
    connection_->Disconnect();
  }

  static QString extractGadgetIcon(const std::string& gadget_path,
                                   const QString& dest_dir) {
    ggadget::StringMap map;

    if (!ggadget::Gadget::GetGadgetManifest(gadget_path.c_str(), &map))
      return QString();

    std::string icon = map[ggadget::kManifestIcon];
    if (icon.empty()) return QString();

    ggadget::scoped_ptr<ggadget::FileManagerInterface> fm(
        ggadget::Gadget::GetGadgetFileManagerForLocale(gadget_path.c_str(),
                                                       NULL));

    if (!fm.get()) return QString();

    std::string data;
    fm->ReadFile(icon.c_str(), &data);
    if (data.empty()) return QString();

    QPixmap pixmap;
    if (pixmap.loadFromData(reinterpret_cast<const uchar *>(data.c_str()),
                            static_cast<int>(data.length()))) {
      QString dest = dest_dir + "/icon.png";
      if (pixmap.save(dest, "png"))
        return dest;
    }
    return QString();
  }

  bool installPlasmaApplet(int id) {
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
           QString(root.path() + "/" + pkg_name).toUtf8().data());
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
    data.setAuthor(QString::fromUtf8(author.c_str()));
    data.setImplementationApi("googlegadgets");
    data.setName(QString::fromUtf8(title.c_str()));
    data.setDescription(QString::fromUtf8(description.c_str()));

    // Extract the icon
    QString icon = extractGadgetIcon(path, root.path());

    Plasma::Package::registerPackage(data, icon);
    return true;
  }

  bool newGadgetInstanceCallback(int id) {
    if (ggadget::qt::ConfirmGadget(gadget_manager_, id)) {
        return installPlasmaApplet(id);
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
    Q_UNUSED(gadget);
    return new GadgetBrowserViewHost(package_, type);
  }

  virtual Gadget *LoadGadget(const char *path, const char *options_name,
                             int instance_id, bool show_debug_console) {
    Q_UNUSED(path);
    Q_UNUSED(options_name);
    Q_UNUSED(instance_id);
    Q_UNUSED(show_debug_console);
    return 0;
  }

  virtual void RemoveGadget(Gadget *gadget, bool save_data) {
    Q_UNUSED(save_data);
    gadget_manager_->RemoveGadgetInstance(gadget->GetInstanceID());
  }

  virtual bool LoadFont(const char *filename) {
    return QFontDatabase::addApplicationFont(filename) != -1;
  }

  virtual void Run() {}
  virtual void ShowGadgetAboutDialog(Gadget*) { }
  virtual void ShowGadgetDebugConsole(Gadget*) {}
  virtual int GetDefaultFontSize() { return ggadget::kDefaultFontSize; }
  virtual bool OpenURL(const Gadget *, const char *) { return false; }

  GadgetManagerInterface *gadget_manager_;
  GglPackage *package_;
  Connection *connection_;
};

GglPackage::GglPackage(QObject *parent, const QVariantList &args)
  : Plasma::PackageStructure(parent), host_(NULL) {
  Q_UNUSED(args);

  setDefaultMimetypes(QStringList() << "application/zip"
                      << "application/x-googlegadget" );
}

GglPackage::~GglPackage() {
  delete host_;
}

bool GglPackage::installPackage(const QString &archive_path,
                                const QString &package_root) {
  Q_UNUSED(package_root);
  ASSERT(!host_);
  host_ = new GadgetBrowserHost(this);
  if (!host_ || !host_->gadget_manager_) {
    delete host_;
    host_ = NULL;
    return false;
  }

  int result = host_->gadget_manager_->NewGadgetInstanceFromFile(
      archive_path.toUtf8().data());

  delete host_;
  host_ = NULL;

  if (result == -1)
    return false;
  else
    return true;
}

void GglPackage::createNewWidgetBrowser(QWidget *parent) {
  ASSERT(!host_);
  host_ = new GadgetBrowserHost(this);
  if (!host_ || !host_->gadget_manager_) {
    gadgetBrowserClosed(); // Actually, it's never opened
    return;
  }
  GetGadgetManager()->ShowGadgetBrowserDialog(host_);
}

void GglPackage::gadgetBrowserClosed() {
  delete host_;
  host_ = NULL;
  emit newWidgetBrowserFinished();
}
