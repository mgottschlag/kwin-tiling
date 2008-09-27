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

#include <QtGui/QMessageBox>

#include <ggadget/gadget_consts.h>
#include <ggadget/gadget.h>
#include <ggadget/messages.h>
#include <ggadget/menu_interface.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/qt/qt_view_widget.h>

#include <plasma/applet.h>
#include "panel_decorator.h"

namespace ggadget {

class PanelDecorator::Private {
 public:
  Private(GadgetInfo *info) : owner_(NULL), info_(info){}

  void ShowDebugInfo(const char*) {
    QString msg = "Applet size:(%1, %2)\nWidget size:(%3, %4)\nView size:(%5, %6)\n";
    qt::QtViewWidget *widget = static_cast<qt::QtViewWidget*>(info_->main_view_host->GetNativeWidget());
    ViewInterface *view = info_->main_view_host->GetViewDecorator();
    QMessageBox::information(NULL,
                             "Debug",
                             msg.arg(info_->applet->size().width())
                             .arg(info_->applet->size().height())
                             .arg(widget->size().width())
                             .arg(widget->size().height())
                             .arg(view->GetWidth())
                             .arg(view->GetHeight()));
  }
  void ShowIcon(const char*) {
    owner_->SetMinimizedIconVisible(!owner_->IsMinimizedIconVisible());
  }
  void ShowCaption(const char*) {
    owner_->SetMinimizedCaptionVisible(!owner_->IsMinimizedCaptionVisible());
  }
  void OnAddDecoratorMenuItems(MenuInterface *menu) {
    int priority = MenuInterface::MENU_ITEM_PRI_DECORATOR;
    owner_->AddCollapseExpandMenuItem(menu);
    if (owner_->IsMinimized()) {
      menu->AddItem("Show Icon", 
                    owner_->IsMinimizedIconVisible()?MenuInterface::MENU_ITEM_FLAG_CHECKED:0,
                    0,
                    NewSlot(this, &Private::ShowIcon), priority);
      menu->AddItem("Show Caption", 
                    owner_->IsMinimizedCaptionVisible()?MenuInterface::MENU_ITEM_FLAG_CHECKED:0,
                    0,
                    NewSlot(this, &Private::ShowCaption), priority);
    }

    menu->AddItem(
        "Debug", 0, 0,
        NewSlot(this, &Private::ShowDebugInfo), priority);
  }
  PanelDecorator *owner_;
  GadgetInfo *info_;
};

PanelDecorator::PanelDecorator(ViewHostInterface *host, GadgetInfo *info)
    : DockedMainViewDecorator(host), d(new Private(info)) {
  SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON, false);
  d->owner_ = this;
}

PanelDecorator::~PanelDecorator() {}

void PanelDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
  d->OnAddDecoratorMenuItems(menu);
}


} // namespace ggadget
