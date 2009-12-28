/*
  Copyright 2009 Google Inc.

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
#include "popout_decorator.h"

#include <QtGui/QMessageBox>

#include <ggadget/gadget_consts.h>
#include <ggadget/gadget.h>
#include <ggadget/messages.h>
#include <ggadget/menu_interface.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/qt/qt_view_widget.h>

#include <Plasma/Applet>

namespace ggadget {

PopOutDecorator::PopOutDecorator(PlasmaViewHost *host)
    : FloatingMainViewDecorator(host, true),
      info(host->getInfo()) {
  SetOptionPrefix("plasma_popout_main_view");
  SetButtonVisible(MainViewDecoratorBase::POP_IN_OUT_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::MENU_BUTTON, false);
  SetButtonVisible(MainViewDecoratorBase::CLOSE_BUTTON, false);
}

PopOutDecorator::~PopOutDecorator() {}

void PopOutDecorator::OnAddDecoratorMenuItems(MenuInterface *menu) {
  AddZoomMenuItem(menu);
}

bool PopOutDecorator::ShowDecoratedView(bool modal, int flags,
                                        Slot1<bool, int> *feedback_handler) {
  info->applet->setMaximumSize(QSizeF());
  return FloatingMainViewDecorator::ShowDecoratedView(modal, flags, feedback_handler);
}

} // namespace ggadget
