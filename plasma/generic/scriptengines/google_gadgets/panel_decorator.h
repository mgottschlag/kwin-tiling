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

#ifndef GGADGET_PANEL_DECORATOR_H__
#define GGADGET_PANEL_DECORATOR_H__

#include <ggadget/docked_main_view_decorator.h>
#include "plasma_view_host.h"

namespace ggadget {

class PanelDecorator : public DockedMainViewDecorator {
 public:
  PanelDecorator(PlasmaViewHost *host, bool vertical);
  virtual ~PanelDecorator();
  virtual void OnAddDecoratorMenuItems(MenuInterface *menu);

 private:
  class Private;
  Private *d;
  void SetVertical(bool vertical);
  DISALLOW_EVIL_CONSTRUCTORS(PanelDecorator);
};

} // namespace ggadget

#endif
