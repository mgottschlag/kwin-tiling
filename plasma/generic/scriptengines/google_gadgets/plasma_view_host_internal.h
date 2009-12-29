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

#ifndef HOSTS_PLASMA_VIEW_HOST_INTERNAL_H__
#define HOSTS_PLASMA_VIEW_HOST_INTERNAL_H__
#include <Plasma/Dialog>
#include <Plasma/Applet>
#include <ggadget/qt/qt_menu.h>
#include <ggadget/view_interface.h>
#include <ggadget/qt/utilities.h>
#include <QDialogButtonBox>
#include <QVBoxLayout>
namespace ggadget{

class PlasmaViewHost::Private : public QObject {
  Q_OBJECT
 public:
  Private(GadgetInfo *i, Type type, bool popout)
    : view_(NULL),
      parent_widget_(NULL),
      widget_(NULL),
      type_(type),
      info(i),
      is_popout_(popout),
      feedback_handler_(NULL) {}

  ~Private() {
    closeView();
    delete feedback_handler_;
  }

  /* Show the view in right place
   *    - floating and docked main view: Shown within the applet
   *    - popouted main view and details view: Shown in QtViewWidget
   *    - OptionsView isn't handled here.
   */
  bool showView(bool modal, int flags, Slot1<bool, int> *feedback_handler) {
    // Only OptionsView needs these two.
    Q_UNUSED(modal);
    Q_UNUSED(flags);

    ASSERT(view_);
    if (feedback_handler_ != feedback_handler) {
      delete feedback_handler_;
      feedback_handler_ = feedback_handler;
    }

    if (widget_) return true;

    if (type_ == ViewHostInterface::VIEW_HOST_MAIN && !is_popout_) {
      // normal main view
      if (info->main_view_widget == NULL) {
        widget_ = new QtViewWidget(view_, 0);
        widget_->setAttribute(Qt::WA_NoSystemBackground);
        info->proxy = new QGraphicsProxyWidget(info->applet);
        info->proxy->setWidget(widget_);
        widget_->show();
        info->main_view_widget = widget_;
      } else {
        widget_ = info->main_view_widget;
        widget_->SetView(view_);
        adjustAppletSize();
      }
      info->applet->setBackgroundHints(Plasma::Applet::NoBackground);
      if (info->applet->location() == Plasma::Floating) {
        connect(widget_, SIGNAL(moved(int, int)),
                this, SLOT(onViewMoved(int, int)));
      } else {
        disconnect();
      }
    } else {
      // Popouted main view and details view
      widget_ = new QtViewWidget(view_, QtViewWidget::FLAG_MOVABLE);
      parent_widget_ = widget_;
      SetGadgetWindowIcon(widget_, view_->GetGadget());
      if (info->expanded_main_view_host &&
          type_ == ViewHostInterface::VIEW_HOST_DETAILS) {
        int w = view_->GetWidth();
        int h = view_->GetHeight();
        QWidget *expanded =
            static_cast<QWidget*>(info->expanded_main_view_host->GetNativeWidget());
        QPoint p = ggadget::qt::GetPopupPosition(expanded->geometry(), QSize(w, h));
        widget_->move(p);
      } else {
        widget_->move(info->applet->popupPosition(widget_->sizeHint()));
      }
      widget_->show();
    }
    return true;
  }

  void closeView() {
    kDebug() << "CloseView";
    if (parent_widget_) {
      delete parent_widget_;
      parent_widget_ = NULL;
      widget_ = NULL;
    } else {
      if (info->applet && widget_) {
        // widget_ is owned by applet, so if applet is null, widget_ is
        // destroyed already
        widget_->SetView(NULL);
      }
      widget_ = NULL;
    }
  }

  void queueDraw() {
    if (parent_widget_) {
      parent_widget_->update();
    } else if (info->applet) {
      info->applet->update();
    }
  }

  // This is called when view size has changed, caused by constraintsEvent or
  // user manually resizes gadget. Applet and widget size will be adjusted
  // according to view size.
  // size changing has two sequences:
  //   view->applet, widget, proxy
  //   applet->view->widget, proxy
  void adjustAppletSize() {
    if (!info->main_view_host || !info->applet || !info->proxy || !widget_) {
      return;
    }
    ViewInterface *view = info->main_view_host->GetViewDecorator();
    double w = view->GetWidth();
    double h = view->GetHeight();
    if (w <= 0 || h <= 0) return;

    kDebug() << "view size:" << w << " " << h;
    kDebug() << "applet old size:" << info->applet->size();
    kDebug() << "widget old size:" << widget_->size();
    kDebug() << "proxy old size:" << info->proxy->size();

    info->applet->resize(w, h);
    info->proxy->resize(w, h);
    widget_->resize(w, h);

    kDebug() << "applet new size:" << info->applet->size();
    kDebug() << "widget new size:" << widget_->size();
    kDebug() << "proxy new size:" << info->proxy->size();
  }

  void queueResize() {
    if (type_ == ViewHostInterface::VIEW_HOST_MAIN && !is_popout_) {
      adjustAppletSize();
    } else if (widget_) {
      widget_->AdjustToViewSize();
    }
  }

  bool showContextMenu(int button) {
    ASSERT(view_);
    Q_UNUSED(button);
    context_menu_.clear();
    QtMenu qt_menu(&context_menu_);
    view_->OnAddContextMenuItems(&qt_menu);
    if (!context_menu_.isEmpty()) {
      context_menu_.popup(QCursor::pos());
      return true;
    } else {
      return false;
    }
  }

  ViewInterface *view_;
  QWidget *parent_widget_;
  QtViewWidget *widget_;
  ViewHostInterface::Type type_;
  GadgetInfo *info;
  bool is_popout_;

  Slot1<bool, int> *feedback_handler_;
  QString caption_;
  QMenu context_menu_;

 private slots:
  void onViewMoved(int x, int y) {
    if (type_ == ViewHostInterface::VIEW_HOST_MAIN && !is_popout_ &&
        info->applet->immutability() == Plasma::Mutable)
      info->applet->moveBy(x, y);
  }
};

} // namespace ggadget

#endif
