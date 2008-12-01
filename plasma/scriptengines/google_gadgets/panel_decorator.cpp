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

#include <Plasma/Applet>
#include "panel_decorator.h"

namespace ggadget {

class PanelDecorator::Private {
 public:
  Private(GadgetInfo *info)
      : owner_(NULL), info_(info), minimized_width_(0), vertical_(true) {}

  void onAddDecoratorMenuItems(MenuInterface *menu) {
    int priority = MenuInterface::MENU_ITEM_PRI_DECORATOR;
    owner_->AddCollapseExpandMenuItem(menu);
    if (owner_->IsMinimized()) {
      menu->AddItem("Show Icon",
                    owner_->IsMinimizedIconVisible() ?
                    MenuInterface::MENU_ITEM_FLAG_CHECKED:0,
                    0,
                    NewSlot(this, &Private::showIcon), priority);
      menu->AddItem("Show Caption",
                    owner_->IsMinimizedCaptionVisible() ?
                    MenuInterface::MENU_ITEM_FLAG_CHECKED:0,
                    0,
                    NewSlot(this, &Private::showCaption), priority);
    }

    menu->AddItem(
        "Debug", 0, 0,
        NewSlot(this, &Private::showDebugInfo), priority);
  }

  void showDebugInfo(const char*) {
    QString msg = "Applet size:(%1, %2)\n"
                  "Widget size:(%3, %4)\n"
                  "View size:(%5, %6)\n"
                  "Aspect:(%7)";
    qt::QtViewWidget *widget = static_cast<qt::QtViewWidget*>(
        info_->main_view_host->GetNativeWidget());
    ViewInterface *view = info_->main_view_host->GetViewDecorator();
    QMessageBox::information(NULL,
                             "Debug",
                             msg.arg(info_->applet->size().width())
                             .arg(info_->applet->size().height())
                             .arg(widget->size().width())
                             .arg(widget->size().height())
                             .arg(view->GetWidth())
                             .arg(view->GetHeight())
                             .arg(info_->applet->aspectRatioMode()));
  }

  void showIcon(const char*) {
    bool caption = owner_->IsMinimizedCaptionVisible();
    owner_->SetMinimizedIconVisible(!owner_->IsMinimizedIconVisible());
    if (caption != owner_->IsMinimizedCaptionVisible())
      updateIconizeStatus();
  }

  void showCaption(const char*) {
    owner_->SetMinimizedCaptionVisible(!owner_->IsMinimizedCaptionVisible());
    updateIconizeStatus();
  }

  void updateIconizeStatus() {
    if (vertical_ || !owner_->IsMinimized()) return;
    if (!owner_->IsMinimizedCaptionVisible()) {
      minimized_width_ = owner_->GetWidth();
      owner_->SetWidth(38);
      owner_->SetResizeBorderVisible(0);
    } else {
      owner_->SetWidth(minimized_width_);
      owner_->SetResizeBorderVisible(BORDER_RIGHT);
    }
  }

  void loadMinimizedWidth() {
    DLOG("LoadMinimizedWidth:");
    Variant width = owner_->GetOption("minimized_width");
    if (width.type() == Variant::TYPE_DOUBLE) {
      minimized_width_ = VariantValue<double>()(width);
      DLOG("\t%f", VariantValue<double>()(width));
    }
    updateIconizeStatus();
  }
  PanelDecorator *owner_;
  GadgetInfo *info_;
  double minimized_width_;
  bool vertical_;
};

PanelDecorator::PanelDecorator(PlasmaViewHost *host)
    : DockedMainViewDecorator(host), d(new Private(host->getInfo())) {
  SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON, false);
  SetOptionPrefix("plasma_panel");
  d->owner_ = this;
}

PanelDecorator::~PanelDecorator() {
  delete d;
}

void PanelDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
  d->onAddDecoratorMenuItems(menu);
}

void PanelDecorator::SetSize(double width, double height) {
  DockedMainViewDecorator::SetSize(width, height);
  if (IsMinimized() && IsMinimizedCaptionVisible()) {
    SetOption("minimized_width", Variant(GetWidth()));
    DLOG("SaveMinimizedWidth:%f", GetWidth());
  }
}

void PanelDecorator::SetResizable(ViewInterface::ResizableMode resizable) {
  View::SetResizable(RESIZABLE_FALSE);
}

/*void PanelDecorator::GetClientExtents(double *width, double *height) const {
  MainViewDecoratorBase::GetClientExtents(width, height);
  if (IsMinimized()) {
    if (!IsMinimizedCaptionVisible())
      *width = 38;
    else
      *width = d->minimized_width_;
  }
}*/

void PanelDecorator::OnChildViewChanged() {
  DockedMainViewDecorator::OnChildViewChanged();
  // this methods is called not only when a main view assigned to this
  // decorator for the first time, but also when a main view popped in/out.
  // We only want to init minimized_width_ the first time
  if (d->minimized_width_ == 0)
    d->loadMinimizedWidth();
}

bool PanelDecorator::ShowDecoratedView(bool modal, int flags,
                       Slot1<bool, int> *feedback_handler) {
  d->info_->applet->setMaximumSize(QSizeF());
  if (d->vertical_)
    d->info_->applet->setMaximumHeight(GetHeight());
  else
    d->info_->applet->setMaximumWidth(GetWidth());
  return DockedMainViewDecorator::ShowDecoratedView(
      modal, flags, feedback_handler);
}

void PanelDecorator::setVertical() {
  SetAllowYMargin(false);
  SetAllowXMargin(true);

  // Gadget on vertical panel is not minimized by default
  Variant vertical_minimized = GetOption("vertical_minimized");
  if (vertical_minimized.type() != Variant::TYPE_BOOL ||
      !VariantValue<bool>()(vertical_minimized)) {
    SetMinimized(false);
  } else {
    SetMinimized(true);
  }

  SetResizeBorderVisible(IsMinimized() ? 0 : BORDER_BOTTOM);
  d->vertical_ = true;
}

void PanelDecorator::setHorizontal() {
  SetAllowYMargin(true);
  SetAllowXMargin(false);

  // Gadget on horizontal panel is minimized by default
  Variant horizontal_minimized = GetOption("horizontal_minimized");
  if (horizontal_minimized.type() != Variant::TYPE_BOOL ||
      VariantValue<bool>()(horizontal_minimized)) {
    SetMinimized(true);
  } else {
    SetMinimized(false);
  }

  SetResizeBorderVisible((!IsMinimized() || IsMinimizedCaptionVisible()) ?
                         0 : BORDER_RIGHT);
  d->vertical_ = false;
}

} // namespace ggadget
