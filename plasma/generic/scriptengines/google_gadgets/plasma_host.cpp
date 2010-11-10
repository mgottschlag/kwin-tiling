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
#include "plasma_host.h"

#include <string>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QtGui/QFontDatabase>
#include <ggadget/common.h>
#include <ggadget/logger.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/script_runtime_manager.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/docked_main_view_decorator.h>
#include <ggadget/details_view_decorator.h>
#include <ggadget/permissions.h>
#include <ggadget/qt/utilities.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/gadget.h>

#include <KDebug>

#include <Plasma/Applet>
#include <Plasma/ToolTipManager>
#include "plasma_view_host.h"
#include "panel_decorator.h"
#include "popout_decorator.h"
#include "floating_decorator.h"

namespace ggadget {

class PlasmaHost::Private {
 public:
  Private(GadgetInfo *i)
      : info(i),
        gadget_w_(0),
        gadget_h_(0) {
    global_permissions_.SetGranted(Permissions::ALL_ACCESS, true);
  }

  void onCloseMainViewHandler() {
    if (info->expanded_main_view_host)
      onPopInHandler();
    info->gadget->RemoveMe(true);
  }

  void onCloseDetailsViewHandler() {
    info->gadget->CloseDetailsView();
  }

  void onClosePopOutViewHandler() {
    onPopInHandler();
  }

  void onPopOutHandler() {
    if (info->expanded_main_view_host != NULL) {
      onPopInHandler();
      return;
    }
    ViewInterface *child = info->main_view_host->GetView();
    ASSERT(child);
    if (child) {
      PlasmaViewHost *vh = new PlasmaViewHost(
              info, ViewHostInterface::VIEW_HOST_MAIN, true);
      PopOutDecorator *view_decorator = new PopOutDecorator(vh);
      DecoratedViewHost *dvh = new DecoratedViewHost(view_decorator);
      view_decorator->ConnectOnClose(
          NewSlot(this, &Private::onClosePopOutViewHandler));

      // Send popout event to decorator first.
      SimpleEvent event(Event::EVENT_POPOUT);
      info->main_view_host->GetViewDecorator()->OnOtherEvent(event);

      child->SwitchViewHost(dvh);
      dvh->ShowView(false, 0, NULL);

      info->expanded_main_view_host = dvh;
    }
  }

  void onPopInHandler() {
    if (!info->expanded_main_view_host) return;
    ViewInterface *child = info->expanded_main_view_host->GetView();
    ASSERT(child);
    if (child) {
      // Close details view
      child->GetGadget()->CloseDetailsView();

      child->SwitchViewHost(info->main_view_host);
      SimpleEvent event(Event::EVENT_POPIN);
      info->main_view_host->GetViewDecorator()->OnOtherEvent(event);
      info->expanded_main_view_host->Destroy();
      info->expanded_main_view_host = NULL;
    }
  }

  DecoratedViewHost *newFloatingViewHost() {
    PlasmaViewHost* vh =  new PlasmaViewHost(
        info, ViewHostInterface::VIEW_HOST_MAIN);

    FloatingDecorator *decorator = new FloatingDecorator(vh);
    decorator->ConnectOnClose(NewSlot(this, &Private::onCloseMainViewHandler));
    decorator->ConnectOnPopOut(NewSlot(this, &Private::onPopOutHandler));
    decorator->ConnectOnPopIn(NewSlot(this, &Private::onPopInHandler));
    DecoratedViewHost *dvh = new DecoratedViewHost(decorator);

    DLOG("NewViewHost: dvh(%p), pvh(%p), vd(%p)",
         dvh, vh, decorator);
    return dvh;
  }

  DecoratedViewHost *newPanelViewHost(bool vertical) {
    PlasmaViewHost* vh =  new PlasmaViewHost(
        info, ViewHostInterface::VIEW_HOST_MAIN);

    PanelDecorator *decorator = new PanelDecorator(vh, vertical);
    decorator->ConnectOnPopOut(NewSlot(this, &Private::onPopOutHandler));
    decorator->ConnectOnPopIn(NewSlot(this, &Private::onPopInHandler));
    DecoratedViewHost *dvh = new DecoratedViewHost(decorator);

    DLOG("NewViewHost: dvh(%p), pvh(%p), vd(%p)",
         dvh, vh, decorator);
    return dvh;
  }

  GadgetInfo *info;
  Permissions global_permissions_;
  double gadget_w_, gadget_h_;
};

PlasmaHost::PlasmaHost(GadgetInfo *info)
  : d(new Private(info)) {
}

PlasmaHost::~PlasmaHost() {
  delete d;
}

ViewHostInterface *PlasmaHost::NewViewHost(Gadget *,
                                           ViewHostInterface::Type type) {
  if (type == ViewHostInterface::VIEW_HOST_MAIN) {
    Plasma::Location loc = d->info->applet->location();
    if (loc == Plasma::Floating) {
      d->info->main_view_host = d->newFloatingViewHost();
    } else {
      d->info->main_view_host = d->newPanelViewHost(isVertical(loc));
    }
    return d->info->main_view_host;
  } else if (type == ViewHostInterface::VIEW_HOST_OPTIONS) {
    ViewHostInterface* vh =  new QtViewHost(type, 1.0, 0, 0, NULL);
    d->info->options_view_host = vh;
    return vh;
  } else {
    ViewHostInterface* vh =  new PlasmaViewHost(d->info, type);
    DetailsViewDecorator *view_decorator = new DetailsViewDecorator(vh);
    DecoratedViewHost *dvh = new DecoratedViewHost(view_decorator);
    view_decorator->ConnectOnClose(
        NewSlot(d, &Private::onCloseDetailsViewHandler));
    d->info->details_view_host = dvh;
    return dvh;
  }
}

void PlasmaHost::RemoveGadget(Gadget *gadget, bool save_data) {
  // Please close me through plasma's button
  Q_UNUSED(gadget);
  Q_UNUSED(save_data);
}

bool PlasmaHost::LoadFont(const char *filename) {
  if (QFontDatabase::addApplicationFont(filename) != -1)
    return true;
  else
    return false;
}

int PlasmaHost::GetDefaultFontSize() {
  return kDefaultFontSize;
}

bool PlasmaHost::OpenURL(const ggadget::Gadget *gadget, const char *url) {
    return ggadget::qt::OpenURL(gadget, url);
}

Gadget* PlasmaHost::LoadGadget(const char *path, const char *options_name,
                               int instance_id, bool show_debug_console) {
  Q_UNUSED(instance_id);
  Q_UNUSED(show_debug_console);

  Gadget *gadget = new Gadget(this, path, options_name, 0,
                              d->global_permissions_,
                              Gadget::DEBUG_CONSOLE_DISABLED);

  if (!gadget->IsValid()) {
    LOG("Failed to load gadget %s", path);
    delete gadget;
    return NULL;
  }

  if (!gadget->ShowMainView()) {
    LOG("Failed to show main view of gadget %s", path);
    delete gadget;
    d->info->main_view_host = NULL;
    return NULL;
  }

  if (gadget->HasOptionsDialog()) {
    d->info->script->setHasConfigurationInterface(true);
  }

  return gadget;
}

void PlasmaHost::onConstraintsEvent(Plasma::Constraints constraints) {
  if (!d->info->main_view_host) return;

  if (constraints & Plasma::FormFactorConstraint) {
    // TODO: Do something to handle it right
    kDebug() << "FormFactorConstraint changed:" << d->info->applet->formFactor();
  }

  if ((constraints & Plasma::LocationConstraint) &&
      d->info->applet->location() != d->info->location) {
    // disable tip.
    // It will be enabled when PanelDecorator is on horizontal panel.
    Plasma::ToolTipManager::self()->unregisterWidget(d->info->applet);

    d->onPopInHandler();
    d->onCloseDetailsViewHandler();
    Plasma::Location loc = d->info->applet->location();

    kDebug() << "LocationConstraint changed from " << d->info->location
             << " to " << loc;

    DecoratedViewHost *vh;
    if (loc == Plasma::Floating) {
      vh = d->newFloatingViewHost();
    } else {
      vh = d->newPanelViewHost(isVertical(loc));
    }

    // Send popout event here so elements like browser_element will know
    // about it and they will hide themselves.
    SimpleEvent event(Event::EVENT_POPOUT);
    d->info->main_view_host->GetViewDecorator()->OnOtherEvent(event);

    ViewInterface *child = d->info->main_view_host->GetView();
    ViewHostInterface *old = child->SwitchViewHost(vh);
    old->Destroy();

    d->info->main_view_host = vh;
    SimpleEvent event1(Event::EVENT_POPIN);
    vh->GetViewDecorator()->OnOtherEvent(event1);

    // Must call it to get the aspectRatioMode of applet right.
    // Maybe we can do it nicely in GGL.
    vh->GetViewDecorator()->GetViewHost()->SetResizable(
            vh->GetViewDecorator()->GetResizable());

    vh->ShowView(false, 0, NULL);
    d->info->location = loc;
    return;
  }

  if (constraints & Plasma::SizeConstraint) {
    ViewInterface *view = d->info->main_view_host->GetViewDecorator();
    if (!view || !d->info->main_view_widget || !d->info->proxy) return;

    QSizeF s = d->info->applet->size();
    kDebug() << "size requested:" << s;
    double w = s.width();
    double h = s.height();
    double old_w = view->GetWidth();
    double old_h = view->GetHeight();
    if (w == old_w && h == old_h) {
      d->info->main_view_widget->resize(w, h);
      d->info->proxy->resize(s);
      return;
    }

    if (view->OnSizing(&w, &h)) {
      view->SetSize(w, h);
    }
  }
}

} // namespace ggadget
