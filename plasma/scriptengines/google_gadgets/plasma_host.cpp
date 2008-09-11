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
#include <ggadget/floating_main_view_decorator.h>
#include <ggadget/docked_main_view_decorator.h>
#include <ggadget/popout_main_view_decorator.h>
#include <ggadget/details_view_decorator.h>
#include <ggadget/permissions.h>
#include <ggadget/qt/utilities.h>
#include <ggadget/gadget.h>

#include <plasma/applet.h>
#include "plasma_view_host.h"
#include "plasma_host.h"
#include "panel_decorator.h"

namespace ggadget {

static const double kConstraint = 99999;

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

  void OnCloseMainViewHandler() {
    if (info->expanded_main_view_host)
      OnPopInHandler();
    info->gadget->RemoveMe(true);
  }

  void OnCloseDetailsViewHandler() {
    info->gadget->CloseDetailsView();
  }

  void OnClosePopOutViewHandler() {
    OnPopInHandler();
  }

  void OnPopOutHandler() {
    if (info->expanded_main_view_host != NULL) {
      OnPopInHandler();
      return;
    }
    ViewInterface *child = info->main_view_host->GetView();
    ASSERT(child);
    if (child) {
      PlasmaViewHost *vh =
          new PlasmaViewHost(info, ViewHostInterface::VIEW_HOST_MAIN, true);
      PopOutMainViewDecorator *view_decorator =
          new PopOutMainViewDecorator(vh);
      DecoratedViewHost *dvh = new DecoratedViewHost(view_decorator);
      view_decorator->ConnectOnClose(
          NewSlot(this, &Private::OnClosePopOutViewHandler));

      // Send popout event to decorator first.
      SimpleEvent event(Event::EVENT_POPOUT);
      info->main_view_host->GetViewDecorator()->OnOtherEvent(event);

      child->SwitchViewHost(dvh);
      dvh->ShowView(false, 0, NULL);

      info->expanded_main_view_host = dvh;
    }
  }

  void OnPopInHandler() {
    kDebug() << "OnPopInHandler";
    ViewInterface *child = info->expanded_main_view_host->GetView();
    ASSERT(child);
    if (child) {
      child->SwitchViewHost(info->main_view_host);
      SimpleEvent event(Event::EVENT_POPIN);
      info->main_view_host->GetViewDecorator()->OnOtherEvent(event);
      info->expanded_main_view_host->Destroy();
      info->expanded_main_view_host = NULL;
    }
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
  ViewHostInterface* vh =  new PlasmaViewHost(d->info, type);

  DecoratedViewHost *dvh;
  if (type == ViewHostInterface::VIEW_HOST_MAIN) {
    if (d->info->is_floating) {
      FloatingMainViewDecorator *decorator =
          new FloatingMainViewDecorator(vh, true);
      dvh = new DecoratedViewHost(decorator);
      decorator->ConnectOnClose(NewSlot(d, &Private::OnCloseMainViewHandler));
      decorator->ConnectOnPopOut(NewSlot(d, &Private::OnPopOutHandler));
      decorator->ConnectOnPopIn(NewSlot(d, &Private::OnPopInHandler));
      decorator->SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON,
                                  false);
      decorator->SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON,
                                  false);
      decorator->SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON,
                                  false);
    } else {
      PanelDecorator *decorator = new PanelDecorator(vh);
      decorator->ConnectOnPopOut(NewSlot(d, &Private::OnPopOutHandler));
      decorator->ConnectOnPopIn(NewSlot(d, &Private::OnPopInHandler));
      dvh = new DecoratedViewHost(decorator);
    }
    d->info->main_view_host = dvh;
    return dvh;
  } else if (type == ViewHostInterface::VIEW_HOST_OPTIONS) {
    d->info->options_view_host = vh;
    return vh;
  } else {
    DetailsViewDecorator *view_decorator = new DetailsViewDecorator(vh);
    dvh = new DecoratedViewHost(view_decorator);
    view_decorator->ConnectOnClose(
        NewSlot(d, &Private::OnCloseDetailsViewHandler));
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

Gadget* PlasmaHost::LoadGadget(const char *path, const char *options_name) {
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
    return NULL;
  }

  if (gadget->HasOptionsDialog()) {
    d->info->script->setHasConfigurationInterface(true);
  }

  return gadget;
}

int PlasmaHost::GetDefaultFontSize() {
    return kDefaultFontSize;
}

bool PlasmaHost::OpenURL(const ggadget::Gadget *gadget, const char *url) {
    return ggadget::qt::OpenURL(gadget, url);
}

void PlasmaHost::AdjustAppletSize() {
  if (!d->info->main_view_host) return;
  ViewInterface *view = d->info->main_view_host->GetViewDecorator();
  double w = view->GetWidth();
  double h = view->GetHeight();
  if (w <= 0 || h <= 0) return;
  if (d->gadget_w_ == w && d->gadget_h_ == h) return;

  d->gadget_w_ = w;
  d->gadget_h_ = h;
  kDebug() << "view size:" << w << " " << h;

  QtViewWidget *widget = static_cast<QtViewWidget*>(d->info->main_view_host->GetNativeWidget());
  kDebug() << "applet old size:" << d->info->applet->size();
  if (widget) kDebug() << "widget old size:" << widget->size();
  d->info->applet->resize(w, h);
  if (widget) {
    widget->AdjustToViewSize();
    widget->resize(w, h);
  }
  kDebug() << "applet new size:" << d->info->applet->size();
  if (widget) kDebug() << "widget new size:" << widget->size();
}

void PlasmaHost::OnConstraintsEvent(Plasma::Constraints constraints) {
  if (!d->info->main_view_host) return;
  ViewInterface *view = d->info->main_view_host->GetViewDecorator();

  if (constraints & Plasma::FormFactorConstraint) {
    kDebug() << "FormFactorConstraint changed";
  }
  if (constraints & Plasma::SizeConstraint) {
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
