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

#include "plasma_view_host.h"

#include <sys/time.h>

#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QToolTip>

#include <KDebug>
#include <KInputDialog>
#include <KMessageBox>
#include <Plasma/Applet>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

#include <ggadget/file_manager_interface.h>
#include <ggadget/gadget_consts.h>
#include <ggadget/logger.h>
#include <ggadget/decorated_view_host.h>
#include <ggadget/options_interface.h>
#include <ggadget/script_context_interface.h>
#include <ggadget/script_runtime_interface.h>
#include <ggadget/script_runtime_manager.h>
#include <ggadget/qt/qt_graphics.h>
#include <ggadget/qt/utilities.h>

#include "plasma_view_host_internal.h"


using namespace ggadget::qt;
namespace ggadget {

PlasmaViewHost::PlasmaViewHost(GadgetInfo *info, ViewHostInterface::Type type,
                               bool popout)
  : d(new Private(info, type, popout)) {
}

PlasmaViewHost::~PlasmaViewHost() {
  delete d;
}

void PlasmaViewHost::Destroy() {
  delete this;
}

void PlasmaViewHost::SetView(ViewInterface *view) {
  kDebug() << "PlasmaViewHost::SetView:" << this << "," << view;
  d->view_ = view;
}

void *PlasmaViewHost::GetNativeWidget() const {
  return d->widget_;
}

void PlasmaViewHost::ViewCoordToNativeWidgetCoord(
    double x, double y, double *widget_x, double *widget_y) const {
  double zoom = d->view_->GetGraphics()->GetZoom();
  if (widget_x) *widget_x = x * zoom;
  if (widget_y) *widget_y = y * zoom;
}

void PlasmaViewHost::NativeWidgetCoordToViewCoord(
    double x, double y, double *view_x, double *view_y) const {
  double zoom = d->view_->GetGraphics()->GetZoom();
  if (zoom == 0) return;
  if (view_x) *view_x = x / zoom;
  if (view_y) *view_y = y / zoom;
}

void PlasmaViewHost::QueueDraw() {
  d->queueDraw();
}

void PlasmaViewHost::QueueResize() {
  d->queueResize();
}

void PlasmaViewHost::SetResizable(ViewInterface::ResizableMode mode) {
  if (d->type_ != ViewHostInterface::VIEW_HOST_MAIN ||
      d->is_popout_ ||
      !d->info->applet)
      return;
  if (mode == ViewInterface::RESIZABLE_TRUE)
    d->info->applet->setAspectRatioMode(Plasma::IgnoreAspectRatio);
  else
    d->info->applet->setAspectRatioMode(Plasma::KeepAspectRatio);
  kDebug() << "SetResizable:" << mode << d->info->applet->aspectRatioMode();
}

static void UpdateTooltip(Plasma::Applet *applet, const QString &text) {
  if (isHorizontal(applet->location())) {
    Plasma::ToolTipContent data;
    data.setMainText(text);
    Plasma::ToolTipManager::self()->setContent(applet, data);
  }
}

void PlasmaViewHost::SetCaption(const std::string &caption) {
  d->caption_ = QString::fromUtf8(caption.c_str());
  if (d->parent_widget_) {
    d->parent_widget_->setWindowTitle(d->caption_);
  } else {
    UpdateTooltip(d->info->applet, d->caption_);
  }
}

void PlasmaViewHost::SetCursor(ggadget::ViewInterface::CursorType type) {
  Qt::CursorShape shape = ggadget::qt::GetQtCursorShape(type);
  // Up to Qt4.4.3, There is a bug in handling cursor when
  // QGraphicsProxyWidget is involved.
  d->info->applet->setCursor(shape);
  if (d->widget_)
    d->widget_->setCursor(shape);
}

void PlasmaViewHost::ShowTooltip(const std::string &tooltip) {
  QToolTip::showText(QCursor::pos(), QString::fromUtf8(tooltip.c_str()));
}

void PlasmaViewHost::ShowTooltipAtPosition(const std::string &tooltip,
                                           double x, double y) {
  double widget_x = 0, widget_y = 0;
  ViewCoordToNativeWidgetCoord(x, y, &widget_x, &widget_y);
  QToolTip::showText(QPoint(widget_x, widget_y),
                     QString::fromUtf8(tooltip.c_str()));
}

bool PlasmaViewHost::ShowView(bool modal, int flags,
                              Slot1<bool, int> *feedback_handler) {
  if (d->showView(modal, flags, feedback_handler)) {
    if (d->parent_widget_) {
      d->parent_widget_->setWindowTitle(d->caption_);
    } else {
      UpdateTooltip(d->info->applet, d->caption_);
    }
    return true;
  }
  return false;
}

void PlasmaViewHost::CloseView() {
  d->closeView();
}

bool PlasmaViewHost::ShowContextMenu(int button) {
  return d->showContextMenu(button);
}

void PlasmaViewHost::Alert(const ViewInterface *view, const char *message) {
  KMessageBox::information(NULL,message,
                           view->GetCaption().c_str());
}

ViewHostInterface::ConfirmResponse PlasmaViewHost::Confirm(
    const ViewInterface *view, const char *message, bool) {
  int ret = KMessageBox::questionYesNo(NULL,
                                       message,
                                       view->GetCaption().c_str() );
  return ret == KMessageBox::Yes ? CONFIRM_YES : CONFIRM_NO;
}

std::string PlasmaViewHost::Prompt(const ViewInterface *view,
                                   const char *message,
                                   const char *default_value) {
  QString s = KInputDialog::getText(view->GetCaption().c_str(),
                                    message,
                                    default_value);
  return s.toUtf8().data();
}

ViewHostInterface::Type PlasmaViewHost::GetType() const {
  return d->type_;
}

ViewInterface *PlasmaViewHost::GetView() const {
  return d->view_;
}

int PlasmaViewHost::GetDebugMode() const {
  return d->info->view_debug_mode;
}

GadgetInfo *PlasmaViewHost::getInfo() {
  return d->info;
}

} // namespace ggadget
#include "plasma_view_host_internal.moc"
