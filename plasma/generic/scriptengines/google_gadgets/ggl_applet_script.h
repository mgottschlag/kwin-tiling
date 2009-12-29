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

#ifndef GGADGET_GGL_APPLET_SCRIPT_H__
#define GGADGET_GGL_APPLET_SCRIPT_H__

#include <Plasma/AppletScript>

namespace ggadget {
  class Gadget;
  class PlasmaHost;
  class DecoratedViewHost;
  class ViewHostInterface;
  namespace qt {
    class QtViewWidget;
  }
}

namespace Plasma {
  class Applet;
}
class QGraphicsProxyWidget;

class GadgetInfo {
 public:
  GadgetInfo()
      : host(NULL),
        gadget(NULL),
        applet(NULL),
        script(NULL),
        proxy(NULL),
        main_view_widget(NULL),
        main_view_host(NULL),
        expanded_main_view_host(NULL),
        details_view_host(NULL),
        options_view_host(NULL),
        view_debug_mode(0),
        location(Plasma::Floating)
  {}
  ggadget::PlasmaHost *host;
  ggadget::Gadget *gadget;
  Plasma::Applet *applet;
  Plasma::AppletScript *script;
  QGraphicsProxyWidget *proxy;
  ggadget::qt::QtViewWidget *main_view_widget;
  ggadget::DecoratedViewHost *main_view_host;
  ggadget::ViewHostInterface *expanded_main_view_host;
  ggadget::ViewHostInterface *details_view_host;
  ggadget::ViewHostInterface *options_view_host;
  int view_debug_mode;
  Plasma::Location location;
};

class GglAppletScript : public Plasma::AppletScript {
  Q_OBJECT
 public:
  GglAppletScript(QObject *parent, const QVariantList &args);
  virtual ~GglAppletScript();

  virtual bool init();
  virtual void constraintsEvent(Plasma::Constraints constraints);
  virtual void paintInterface(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              const QRect &contentsRect);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual QList<QAction*> contextualActions();

 public Q_SLOTS:
  virtual void showConfigurationInterface();

 private Q_SLOTS:
  void loadGadget();

 private:
  class Private;
  Private *const d;
};

#endif
