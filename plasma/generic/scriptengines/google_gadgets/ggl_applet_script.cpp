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
#include "ggl_applet_script.h"

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

#include <KDebug>

#include <Plasma/Applet>
#include <Plasma/Package>

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
#include "ggl_extensions.h"

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(googlegadget, GglAppletScript)

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

  std::string profile_dir =
          ggadget::BuildFilePath(ggadget::GetHomeDirectory().c_str(),
                                 ".google/gadgets-plasma", NULL);

  QString error;
  if (!ggadget::qt::InitGGL(NULL, "ggl-plasma", profile_dir.c_str(),
                            kGlobalExtensions, 0,
                            ggadget::qt::GGL_INIT_FLAG_COLLECTOR, &error)) {
    kError() << "Failed to init GGL system:" << error;
    return false;
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

  d->info.location = applet()->location();
  d->info.applet = applet();
  d->info.host = new ggadget::PlasmaHost(&d->info);
  d->info.gadget = d->info.host->LoadGadget(d->gg_file_.toUtf8(),
                                            d->options_.toUtf8(),
                                            0, false);
}

void GglAppletScript::paintInterface(QPainter *p,
                                     const QStyleOptionGraphicsItem *option,
                                     const QRect &contentsRect) {
  Q_UNUSED(p);
  Q_UNUSED(option);
  Q_UNUSED(contentsRect);
#if 0
  QRect r = contentsRect;
  p->setPen(QColor(0, 0, 255));
  p->drawLine(r.left(), r.top(), r.right(), r.bottom());
  p->drawLine(r.left(), r.bottom(), r.right(), r.top());
  p->drawRect(r);
#endif
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
    d->info.host->onConstraintsEvent(constraints);
}

void GglAppletScript::showConfigurationInterface() {
  if (d->info.gadget)
    d->info.gadget->ShowOptionsDialog();
}

#include "ggl_applet_script.moc"
