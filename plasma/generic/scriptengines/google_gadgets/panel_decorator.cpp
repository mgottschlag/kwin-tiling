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
#include "panel_decorator.h"

#include <QtGui/QMessageBox>

#include <ggadget/gadget_consts.h>
#include <ggadget/gadget.h>
#include <ggadget/messages.h>
#include <ggadget/menu_interface.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/qt/qt_view_widget.h>

#include <Plasma/Applet>

namespace ggadget {

class PanelDecorator::Private {
 public:
  Private(GadgetInfo *info, bool vertical)
      : owner_(NULL), info_(info), vertical_(vertical) {
  }

  void onAddDecoratorMenuItems(MenuInterface *menu) {
    int priority = MenuInterface::MENU_ITEM_PRI_DECORATOR;
    if (vertical_) {
      owner_->AddCollapseExpandMenuItem(menu);
    }

#ifndef NDEBUG
    menu->AddItem(
        "Debug", 0, 0,
        NewSlot(this, &Private::showDebugInfo), priority);
#endif
  }

#ifndef NDEBUG
  void showDebugInfo(const char*) {
    QString msg = "Applet size:(%1, %2)\n"
                  "Widget size:(%3, %4)\n"
                  "Decorator size:(%5, %6)\n"
                  "View size:(%7, %8)\n"
                  "Aspect:(%9)";
    qt::QtViewWidget *widget = static_cast<qt::QtViewWidget*>(
        info_->main_view_host->GetNativeWidget());
    ViewInterface *decorator = info_->main_view_host->GetViewDecorator();
    ViewInterface *view = info_->main_view_host->GetView();
    QMessageBox::information(NULL,
                             "Debug",
                             msg.arg(info_->applet->size().width())
                             .arg(info_->applet->size().height())
                             .arg(widget->size().width())
                             .arg(widget->size().height())
                             .arg(decorator->GetWidth())
                             .arg(decorator->GetHeight())
                             .arg(view->GetWidth())
                             .arg(view->GetHeight())
                             .arg(info_->applet->aspectRatioMode()));
  }
#endif
  
  PanelDecorator *owner_;
  GadgetInfo *info_;
  bool vertical_;
};

PanelDecorator::PanelDecorator(PlasmaViewHost *host, bool vertical)
    : DockedMainViewDecorator(host),
      d(new Private(host->getInfo(), vertical)) {
  SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON, false);
  SetVertical(vertical);
  d->owner_ = this;
}

PanelDecorator::~PanelDecorator() {
  delete d;
}

void PanelDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
  d->onAddDecoratorMenuItems(menu);
}

void PanelDecorator::SetVertical(bool vertical) {
  if (vertical) {
    SetOptionPrefix("plasma_vpanel");
    SetAllowYMargin(false);
    SetAllowXMargin(true);
    SetWidth(d->info_->applet->size().width());
  } else {
    SetOptionPrefix("plasma_hpanel");
    SetAllowYMargin(true);
    SetAllowXMargin(false);

    // Gadget on horizontal panel is minimized, caption-hidden.
    SetMinimized(true);
    SetMinimizedIconVisible(true);
    SetMinimizedCaptionVisible(false);
    
    SetWidth(38);

    SetResizeBorderVisible(0);
  }
}

} // namespace ggadget
