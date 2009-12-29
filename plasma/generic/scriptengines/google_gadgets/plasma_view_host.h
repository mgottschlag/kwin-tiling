/*
  Copyright 2007 Google Inc.

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

#ifndef GGADGET_PLASMA_VIEW_HOST_H__
#define GGADGET_PLASMA_VIEW_HOST_H__

#include <set>

#include <QtGui/QGraphicsWidget>

#include <ggadget/view_interface.h>
#include <ggadget/graphics_interface.h>
#include <ggadget/view_host_interface.h>
#include <ggadget/qt/qt_graphics.h>
#include <ggadget/qt/qt_view_widget.h>
#include "plasma_host.h"

namespace ggadget {

using namespace ggadget::qt;

inline bool isHorizontal(Plasma::Location loc) {
  return loc == Plasma::TopEdge || loc == Plasma::BottomEdge;
}

inline bool isVertical(Plasma::Location loc) {
  return loc == Plasma::LeftEdge || loc == Plasma::RightEdge;
}

class PlasmaViewHost : public ViewHostInterface {
 public:
  PlasmaViewHost(GadgetInfo *info, ViewHostInterface::Type type, bool popout = false);
  virtual ~PlasmaViewHost();

  virtual Type GetType() const;
  virtual void Destroy();
  virtual void SetView(ViewInterface *view);
  virtual ViewInterface *GetView() const;
  virtual GraphicsInterface *NewGraphics() const {
    return new QtGraphics(1.0);
  }
  virtual void *GetNativeWidget() const;
  virtual void ViewCoordToNativeWidgetCoord(
      double x, double y, double *widget_x, double *widget_y) const;
  virtual void NativeWidgetCoordToViewCoord(
      double x, double y, double *view_x, double *view_y) const;
  virtual void QueueDraw();
  virtual void QueueResize();
  virtual void EnableInputShapeMask(bool enable) { Q_UNUSED(enable); }
  virtual void SetResizable(ViewInterface::ResizableMode mode);
  virtual void SetCaption(const std::string &caption);
  virtual void SetShowCaptionAlways(bool) {}
  virtual void SetCursor(ggadget::ViewInterface::CursorType cursor);
  virtual void ShowTooltip(const std::string &tooltip);
  virtual void ShowTooltipAtPosition(const std::string &tooltip,
                                     double x, double y);
  virtual bool ShowView(bool modal, int flags,
                        Slot1<bool, int> *feedback_handler);
  virtual void CloseView();
  virtual bool ShowContextMenu(int button);
  virtual void BeginResizeDrag(int, ViewInterface::HitTest) {}
  virtual void BeginMoveDrag(int) {}

  virtual void Alert(const ViewInterface *view, const char *message);
  virtual ConfirmResponse Confirm(const ViewInterface *view,
                                  const char *message, bool);
  virtual std::string Prompt(const ViewInterface *view,
                             const char *message,
                             const char *default_value);
  virtual int GetDebugMode() const;

  GadgetInfo *getInfo();

 private:
  class Private;
  Private *d;
  DISALLOW_EVIL_CONSTRUCTORS(PlasmaViewHost);
};

} // namespace ggadget

#endif // GGADGET_PLASMA_VIEW_HOST_H__
