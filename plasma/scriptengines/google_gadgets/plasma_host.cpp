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

#include <string>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QFontDatabase>
#include <ggadget/common.h>
#include <ggadget/logger.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/script_runtime_manager.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/docked_main_view_decorator.h>
#include <ggadget/popout_main_view_decorator.h>
#include <ggadget/details_view_decorator.h>
#include <ggadget/permissions.h>
#include <ggadget/qt/utilities.h>
#include <ggadget/qt/qt_view_host.h>
#include <ggadget/gadget.h>

#include <Plasma/Applet>
#include "plasma_view_host.h"
#include "plasma_host.h"
#include "panel_decorator.h"
#include "floating_decorator.h"

namespace ggadget {

static const double kConstraint = 99999;

static inline bool isHorizontal(Plasma::Location loc) {
  return loc == Plasma::TopEdge || loc == Plasma::BottomEdge;
}

static inline bool isVertical(Plasma::Location loc) {
  return loc == Plasma::LeftEdge || loc == Plasma::RightEdge;
}

class PlasmaHost::Private {
 public:
  Private(GadgetInfo *i)
      : info(i),
        constraint_w_(kConstraint),
        constraint_h_(kConstraint),
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
      ggadget::qt::QtViewHost *vh =
          new ggadget::qt::QtViewHost(
              ViewHostInterface::VIEW_HOST_MAIN, 1.0, 0, 0, NULL);
      PopOutMainViewDecorator *view_decorator =
          new PopOutMainViewDecorator(vh);
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
    ViewHostInterface* vh =  new PlasmaViewHost(
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

  DecoratedViewHost *newPanelViewHost() {
    ViewHostInterface* vh =  new PlasmaViewHost(
        info, ViewHostInterface::VIEW_HOST_MAIN);

    PanelDecorator *decorator = new PanelDecorator(vh, info);
    if (isHorizontal(info->applet->location()))
      decorator->setHorizontal();
    else
      decorator->setVertical();
    decorator->ConnectOnPopOut(NewSlot(this, &Private::onPopOutHandler));
    decorator->ConnectOnPopIn(NewSlot(this, &Private::onPopInHandler));
    DecoratedViewHost *dvh = new DecoratedViewHost(decorator);
    DLOG("NewViewHost: dvh(%p), pvh(%p), vd(%p)",
         dvh, vh, decorator);
    return dvh;
  }

  GadgetInfo *info;
  Permissions global_permissions_;
  double constraint_w_, constraint_h_;
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
    if (d->info->applet->location() == Plasma::Floating) {
      d->info->main_view_host = d->newFloatingViewHost();
    } else {
      d->info->main_view_host = d->newPanelViewHost();
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
}

bool PlasmaHost::LoadFont(const char *filename) {
  if (QFontDatabase::addApplicationFont(filename) != -1)
    return true;
  else
    return false;
}

void PlasmaHost::ShowGadgetAboutDialog(Gadget *gadget) {
  qt::ShowGadgetAboutDialog(gadget);
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
    d->onPopInHandler();
    d->onCloseDetailsViewHandler();
    Plasma::Location loc = d->info->applet->location();

    kDebug() << "LocationConstraint changed from " << d->info->location
             << " to " << loc;

    if ((d->info->location == Plasma::Floating && loc != Plasma::Floating) ||
        (d->info->location != Plasma::Floating && loc == Plasma::Floating)) {
      DecoratedViewHost *vh;
      if (loc == Plasma::Floating)
        vh = d->newFloatingViewHost();
      else
        vh = d->newPanelViewHost();

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
    } else if (isVertical(d->info->location) != isVertical(loc)) {
      PanelDecorator *decorator = static_cast<PanelDecorator*>(
          d->info->main_view_host->GetViewDecorator());
      if (isVertical(loc))
        decorator->setVertical();
      else
        decorator->setHorizontal();
    }
    d->info->location = loc;
    return;
  }

  if (constraints & Plasma::SizeConstraint) {
    ViewInterface *view = d->info->main_view_host->GetViewDecorator();
    if (!view) return;
    QSizeF s = d->info->applet->size();
    kDebug() << "size requested:" << s;
    d->constraint_w_ = s.width();
    d->constraint_h_ = s.height();
    double w = s.width();
    double h = s.height();

    // if gadget is on panel, we don't occupy the whole available space
    if (d->info->applet->formFactor() == Plasma::Horizontal
        && w > view->GetWidth()) {
      w = view->GetWidth();
    }
    if (d->info->applet->formFactor() == Plasma::Vertical
        && h > view->GetHeight()) {
      h = view->GetHeight();
    }

    if (view->OnSizing(&w, &h)) {
      kDebug() << "Original view size:" << view->GetWidth()
               << " " << view->GetHeight();
      view->SetSize(w, h);
      kDebug() << "view size change to:" << w << " " << h;
    }
  }
}

} // namespace ggadget
