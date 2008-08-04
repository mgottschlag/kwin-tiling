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
#include <ggadget/permissions.h>
#include <ggadget/qt/utilities.h>

#include "plasma_view_host.h"
#include "plasma_host.h"

namespace ggadget {
static double kMaxConstraint = 999999;
class PlasmaHost::Private {
 public:
  Private(GadgetInfo *i)
      : info(i),
        constraint_width_(kMaxConstraint),
        constraint_height_(kMaxConstraint) {
    global_permissions_.SetGranted(Permissions::ALL_ACCESS, true);
  }

  void OnCloseHandler(DecoratedViewHost *decorated) {
    switch (decorated->GetDecoratorType()) {
      case DecoratedViewHost::MAIN_STANDALONE:
        if (info->expanded_main_view_host)
          OnPopInHandler(info->main_view_host);
        info->gadget->RemoveMe(true);
        break;
      case DecoratedViewHost::MAIN_EXPANDED:
        OnPopInHandler(info->main_view_host);
        break;
      case DecoratedViewHost::DETAILS:
        info->gadget->CloseDetailsView();
        break;
      default:
        ASSERT("Invalid decorator type.");
    }
  }

  void OnPopOutHandler(DecoratedViewHost *decorated) {
    if (info->expanded_main_view_host != NULL) {
      OnPopInHandler(decorated);
      return;
    }
    ViewInterface *child = decorated->GetView();
    ASSERT(child);
    if (child) {
      PlasmaViewHost *vh =
        new PlasmaViewHost(info, ViewHostInterface::VIEW_HOST_MAIN, true);
      if (info->applet->location() == Plasma::Floating) {
        DecoratedViewHost* dvh =
            new DecoratedViewHost(vh, DecoratedViewHost::MAIN_EXPANDED, true);
        dvh->ConnectOnClose(
            NewSlot(this, &Private::OnCloseHandler, dvh));

        // Send popout event to decorator first.
        SimpleEvent event(Event::EVENT_POPOUT);
        dvh->GetDecoratedView()->OnOtherEvent(event);
        info->expanded_main_view_host = dvh;
      } else {
        info->expanded_main_view_host = vh;
      }

      child->SwitchViewHost(info->expanded_main_view_host);
      info->expanded_main_view_host->ShowView(false, 0, NULL);
    }
  }

  void OnPopInHandler(DecoratedViewHost *decorated) {
    kDebug() << "OnPopInHandler";
    ViewInterface *child = info->expanded_main_view_host->GetView();
    ASSERT(child);
    if (child) {
      child->SwitchViewHost(info->main_view_host);
      if (info->applet->location() == Plasma::Floating) {
        SimpleEvent event(Event::EVENT_POPIN);
        info->main_view_host->GetDecoratedView()->OnOtherEvent(event);
      }
      info->expanded_main_view_host->Destroy();
      info->expanded_main_view_host = NULL;
    }
  }

  GadgetInfo *info;
  Permissions global_permissions_;
  double constraint_width_, constraint_height_;

};

PlasmaHost::PlasmaHost(GadgetInfo *info)
  : d(new Private(info)) {
}

PlasmaHost::~PlasmaHost() {
  delete d;
}

ViewHostInterface *PlasmaHost::NewViewHost(Gadget *gadget,
                                           ViewHostInterface::Type type) {
  ViewHostInterface* vh =
      new PlasmaViewHost(d->info, type);
  if (type == ViewHostInterface::VIEW_HOST_MAIN) {
    DecoratedViewHost *dvh;
    // TODO: We should use different decorator for main views on locations
    // other than Floating. But we don't have proper one.
    if (d->info->applet->location() == Plasma::Floating)
      dvh = new DecoratedViewHost(vh, DecoratedViewHost::MAIN_STANDALONE, true);
    else
      dvh = new DecoratedViewHost(vh, DecoratedViewHost::MAIN_STANDALONE, true);
    dvh->ConnectOnClose(NewSlot(d, &Private::OnCloseHandler, dvh));
    dvh->ConnectOnPopOut(NewSlot(d, &Private::OnPopOutHandler, dvh));
    dvh->ConnectOnPopIn(NewSlot(d, &Private::OnPopInHandler, dvh));
    d->info->main_view_host = dvh;
    return dvh;
  } else if (type == ViewHostInterface::VIEW_HOST_OPTIONS) {
    d->info->options_view_host = vh;
    return vh;
  } else {
    DecoratedViewHost *dvh =
        new DecoratedViewHost(vh, DecoratedViewHost::DETAILS, false);
    dvh->ConnectOnClose(NewSlot(d, &Private::OnCloseHandler, dvh));
    d->info->details_view_host = dvh;
    return dvh;
  }
}

void PlasmaHost::RemoveGadget(Gadget *gadget, bool save_data) {
  // TODO: Right now, please close me through plasma's button
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

  return gadget;
}

void PlasmaHost::AdjustAppletSize() {
  if (!d->info->main_view_host) return;
  ViewInterface *view = d->info->main_view_host->GetDecoratedView();
  int w = static_cast<int>(view->GetWidth());
  int h = static_cast<int>(view->GetHeight());
  if (w > 0 && h > 0) {
    if (w > d->constraint_width_) w = d->constraint_width_;
    if (h > d->constraint_height_) h = d->constraint_height_;
    d->info->applet->resize(w, h);
    kDebug() << "applet size:" << d->info->applet->size();
  }
}

void PlasmaHost::OnConstraintsEvent(Plasma::Constraints constraints) {
  if (!d->info->main_view_host) return;
  ViewInterface *view = d->info->main_view_host->GetDecoratedView();

  if (constraints & Plasma::FormFactorConstraint) {
    kDebug() << "FormFactorConstraint changed";
  }
  if (constraints & Plasma::SizeConstraint) {
    QSizeF s = d->info->applet->size();
    kDebug() << "size requested:" << s;
    d->constraint_width_ = s.width();
    d->constraint_height_ = s.height();
    double w = d->constraint_width_;
    double h = d->constraint_height_;
    if (view->OnSizing(&w, &h)) {
      kDebug() << "Original view size:" << view->GetWidth()
               << " " << view->GetHeight();
      // if gadget is on panel, we don't occupy the whole available space
      if (d->info->applet->formFactor() == Plasma::Horizontal
          && w > view->GetWidth())
        w = view->GetWidth();
      if (d->info->applet->formFactor() == Plasma::Vertical
          && h > view->GetHeight())
        h = view->GetHeight();
      view->SetSize(w, h);
      kDebug() << "view size change to:" << w << " " << h;
      if (d->info->applet->formFactor() == Plasma::Vertical &&
          w > d->constraint_width_)
        w = d->constraint_width_;
      if (d->info->applet->formFactor() == Plasma::Horizontal &&
          h > d->constraint_height_)
        h = d->constraint_height_;

      d->info->applet->resize(w, h);
      kDebug() << "applet size change to:" << w << " " << h;
      if (d->info->applet->formFactor() != Plasma::Horizontal)
        d->constraint_height_ = kMaxConstraint;
      if (d->info->applet->formFactor() != Plasma::Vertical)
        d->constraint_width_ = kMaxConstraint;
    }
  }
}

} // namespace ggadget
