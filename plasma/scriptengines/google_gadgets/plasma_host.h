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

#ifndef GGADGET_PLASMA_HOST_H__
#define GGADGET_PLASMA_HOST_H__

#include <ggadget/host_interface.h>
#include "ggl_applet_script.h"
class Gadget;

namespace ggadget {

using ggadget::ViewHostInterface;

class PlasmaHost : public ggadget::HostInterface {
 public:
  PlasmaHost(GadgetInfo *info);
  virtual ~PlasmaHost();
  virtual ViewHostInterface *NewViewHost(Gadget *gadget,
                                         ViewHostInterface::Type type);
  virtual Gadget *LoadGadget(const char *path, const char *options_name,
                             int instance_id, bool show_debug_console);
  virtual void RemoveGadget(Gadget *gadget, bool save_data);
  virtual bool LoadFont(const char *filename);
  virtual void Run() {}
  virtual void ShowGadgetDebugConsole(Gadget *) {}
  virtual int GetDefaultFontSize();
  virtual bool OpenURL(const Gadget *gadget, const char *url);

  void onConstraintsEvent(Plasma::Constraints constraints);

 private:
  class Private;
  Private *d;
};

} // namespace ggadget

#endif // GGADGET_PLASMA_HOST_H__
