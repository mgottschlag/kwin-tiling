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

namespace ggadget {

PanelDecorator::PanelDecorator(ViewHostInterface *host)
    : DockedMainViewDecorator(host) {
}

PanelDecorator::~PanelDecorator() {}

void PanelDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
/*  int priority = MenuInterface::MENU_ITEM_PRI_DECORATOR;
  menu->AddItem(
      GM_(IsMinimized() ? "MENU_ITEM_EXPAND" : "MENU_ITEM_COLLAPSE"), 0, 0,
      NewSlot(impl_, &Impl::CollapseExpandMenuCallback), priority);*/
}

} // namespace ggadget
