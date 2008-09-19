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

#include <sys/time.h>
#include <time.h>

#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QFileInfo>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <plasma/applet.h>
#include <plasma/package.h>

#include <ggadget/logger.h>
#include <ggadget/script_runtime_interface.h>
#include <ggadget/qt/qt_view_widget.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/qt/qt_menu.h>
#include <ggadget/qt/utilities.h>
#include <ggadget/qt/qt_main_loop.h>
#include <ggadget/extension_manager.h>
#include <ggadget/script_runtime_manager.h>
#include <ggadget/gadget.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/system_utils.h>
#include <ggadget/host_utils.h>
#include <ggadget/view_interface.h>
#include <ggadget/view.h>
#include <ggadget/host_interface.h>
#include <ggadget/decorated_view_host.h>
#include "plasma_host.h"
#include "ggl_applet_script.h"

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(googlegadget, GglAppletScript)

static const char *kGlobalExtensions[] = {
  "default-framework",
  "libxml2-xml-parser",
  "default-options",
  "qtwebkit-browser-element",
  "qt-system-framework",
  "qt-edit-element",
  "phonon-audio-framework",
  "gst-mediaplayer-element",
  "linux-system-framework",
  "qt-xml-http-request",
  NULL
};

static bool g_initialized = false;
static ggadget::qt::QtMainLoop main_loop;
static QMutex mutex;

class GglAppletScript::Private {
 public:
  QString gg_file_;
  QString options_;
  QMenu menu_;
  QStringList errors_;
  GadgetInfo info;
  ~Private() {
    // Must set applet to null so other components could know the applet is
    // exiting.
    info.applet = NULL;
    delete info.host;
    info.host = NULL;
    delete info.gadget;
    info.gadget = NULL;
  }
};

GglAppletScript::GglAppletScript(QObject *parent, const QVariantList &args)
  : Plasma::AppletScript(parent), d(new Private) {
  Q_UNUSED(args);
  d->info.script = this;
}

GglAppletScript::~GglAppletScript() {
  kWarning() << "GGL applet script destroied";
  delete d;
}

bool GglAppletScript::init() {
  Q_ASSERT(applet());
  Q_ASSERT(package());

  if (!g_initialized) {
    QMutexLocker lock(&mutex);
    if (!g_initialized) {
      ggadget::SetGlobalMainLoop(&main_loop);
      ggadget::SetupLogger(ggadget::LOG_TRACE, true);

      std::string profile_dir =
          ggadget::BuildFilePath(ggadget::GetHomeDirectory().c_str(),
                                 ggadget::kDefaultProfileDirectory, NULL);
      ggadget::EnsureDirectories(profile_dir.c_str());

      // Set global file manager.
      ggadget::SetupGlobalFileManager(profile_dir.c_str());

      // Load global extensions.
      ggadget::ExtensionManager *ext_manager =
          ggadget::ExtensionManager::CreateExtensionManager();
      ggadget::ExtensionManager::SetGlobalExtensionManager(ext_manager);

      // Ignore errors when loading extensions.
      for (size_t i = 0; kGlobalExtensions[i]; ++i)
        ext_manager->LoadExtension(kGlobalExtensions[i], false);

      if (!ext_manager->LoadExtension("smjs-script-runtime", false))
        ext_manager->LoadExtension("qt-script-runtime", false);

      // Register JavaScript runtime.
      ggadget::ScriptRuntimeManager *manager =
          ggadget::ScriptRuntimeManager::get();
      ggadget::ScriptRuntimeExtensionRegister script_runtime_register(manager);
      ext_manager->RegisterLoadedExtensions(&script_runtime_register);

      ext_manager->SetReadonly();
      ggadget::InitXHRUserAgent("ggl-plasma");
      g_initialized = true;
    }
  }

  QFile config_file(package()->path() + "/config.txt");
  if (!config_file.open(QIODevice::ReadOnly)) {
    kError() << "Failed to open google gadget's config file at "
             << package()->path();
    return false;
  }
  QTextStream in(&config_file);
  d->gg_file_ = in.readLine();
  d->options_ = in.readLine();
  if (d->options_.isNull() || d->options_.isEmpty())
    return false;

  applet()->setAspectRatioMode(Plasma::ConstrainedSquare);
  QTimer::singleShot(50, this, SLOT(loadGadget()));
  return true;
}

void GglAppletScript::loadGadget() {
  d->errors_.clear();
  kDebug() << "Loading gadget " << d->gg_file_
           << "with options " << d->options_;

  if (applet()->location() != Plasma::Floating) {
    d->info.is_floating = false;
  }
  d->info.applet = applet();
  d->info.host = new ggadget::PlasmaHost(&d->info);
  d->info.gadget = d->info.host->LoadGadget(d->gg_file_.toUtf8(),
                                            d->options_.toUtf8());
}

void GglAppletScript::paintInterface(QPainter *p,
                                     const QStyleOptionGraphicsItem *option,
                                     const QRect &contentsRect) {
}

void GglAppletScript::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  // FIXME: AppletScript has no way to handle mousePressEvent right now
  if (event->button() == Qt::RightButton) {
    kDebug() << "Right button pressed";
    d->menu_.clear();
    ggadget::qt::QtMenu qt_menu(&d->menu_);
    ggadget::ViewInterface *view = d->info.main_view_host->GetViewDecorator();
    if (!view->OnAddContextMenuItems(&qt_menu)) {
      if (!d->menu_.isEmpty()) {
        kDebug() << "Show my own menu";
        d->menu_.exec(event->screenPos());
        event->accept();
      }
    }
  }
}

QList<QAction*> GglAppletScript::contextualActions() {
  d->menu_.clear();
  if (d->info.main_view_host) {
    ggadget::ViewInterface *view = d->info.main_view_host->GetViewDecorator();
    if (view) {
      ggadget::qt::QtMenu qt_menu(&d->menu_);
      view->OnAddContextMenuItems(&qt_menu);
    }
  }
  return d->menu_.actions();
}

void GglAppletScript::constraintsEvent(Plasma::Constraints constraints) {
  if (d->info.host)
    d->info.host->OnConstraintsEvent(constraints);
}

void GglAppletScript::showConfigurationInterface() {
  if (d->info.gadget)
    d->info.gadget->ShowOptionsDialog();
}

#include "ggl_applet_script.moc"
