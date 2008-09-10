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
#include <plasma/dialog.h>
#include <plasma/applet.h>
#include <ggadget/qt/qt_menu.h>
#include <ggadget/view_interface.h>
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
      onoptionchanged_connection_(NULL),
      feedback_handler_(NULL) {}

  ~Private() {
    if (onoptionchanged_connection_)
      onoptionchanged_connection_->Disconnect();
    CloseView();
  }

  static void EmbededWidget(QGraphicsWidget *parent, QWidget *widget) {
    widget->setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(parent);
    layout->setSpacing(0);
    QGraphicsProxyWidget* proxy = new QGraphicsProxyWidget(parent);
    proxy->setWidget(widget);
    layout->addItem(proxy);
    parent->setLayout(layout);
  }
  /* Show the view in right place
   *    - floating main view: Shown within the applet
   *    - popouted main view and details view: Shown in QtViewWidget
   *    - options view: Shown in QDialog
   */
  bool ShowView(bool modal, int flags, Slot1<void, int> *feedback_handler) {
    ASSERT(view_);
    if (feedback_handler_ && feedback_handler_ != feedback_handler)
      delete feedback_handler_;
    feedback_handler_ = feedback_handler;

    if (widget_) return true;

    if (type_ == ViewHostInterface::VIEW_HOST_OPTIONS) {
      // Create a Widget (composite, decorated, movable, input_mask)
      widget_ = new QtViewWidget(view_, false, true, true, false);
      QVBoxLayout *layout = new QVBoxLayout();
      widget_->setFixedSize(D2I(view_->GetWidth()), D2I(view_->GetHeight()));
      layout->addWidget(widget_);

      QDialog *dialog = new QDialog();
      parent_widget_ = dialog;

      QDialogButtonBox::StandardButtons what_buttons = 0;
      if (flags & ViewInterface::OPTIONS_VIEW_FLAG_OK)
        what_buttons |= QDialogButtonBox::Ok;

      if (flags & ViewInterface::OPTIONS_VIEW_FLAG_CANCEL)
        what_buttons |= QDialogButtonBox::Cancel;

      if (what_buttons != 0) {
        QDialogButtonBox *buttons = new QDialogButtonBox(what_buttons);

        if (flags & ViewInterface::OPTIONS_VIEW_FLAG_OK)
          dialog->connect(buttons, SIGNAL(accepted()),
                           this, SLOT(OnOptionViewOK()));
        if (flags & ViewInterface::OPTIONS_VIEW_FLAG_CANCEL)
          dialog->connect(buttons, SIGNAL(rejected()),
                           this, SLOT(OnOptionViewCancel()));
        layout->addWidget(buttons);
      }

      dialog->setLayout(layout);
      dialog->setWindowTitle(caption_);
      SetGadgetWindowIcon(dialog, view_->GetGadget());
      if (modal)
        dialog->exec();
      else
        dialog->show();
    } else if (type_ == ViewHostInterface::VIEW_HOST_DETAILS) {
      widget_ = new QtViewWidget(view_, false, false, true, false);
      parent_widget_ = widget_;
      SetGadgetWindowIcon(widget_, view_->GetGadget());
      widget_->show();
      widget_->move(info->applet->popupPosition(widget_->sizeHint()));
    } else if (!is_popout_) {  // normal main view
      // Create a Widget (composite, decorated, movable, input_mask)
      widget_ = new QtViewWidget(view_, false, true, false, false);
      EmbededWidget(info->applet, widget_);
      info->applet->setBackgroundHints(Plasma::Applet::NoBackground);
      if (info->is_floating)
        connect(widget_, SIGNAL(moved(int, int)),
                this, SLOT(OnViewMoved(int, int)));

      if (info->applet->formFactor() == Plasma::Vertical)
        view_->SetWidth(info->applet->size().width());
      if (info->applet->formFactor() == Plasma::Horizontal)
        view_->SetHeight(info->applet->size().height());
    } else {
      widget_ = new QtViewWidget(view_, false, false, true, false);
      parent_widget_ = widget_;
      SetGadgetWindowIcon(widget_, view_->GetGadget());
      widget_->show();
      widget_->move(info->applet->popupPosition(widget_->sizeHint()));
    }
    return true;
  }

  void CloseView() {
    kDebug() << "CloseView";
    if (parent_widget_) {
      delete parent_widget_;
      parent_widget_ = NULL;
      widget_ = NULL;
    }
  }

  void QueueDraw() {
    if (parent_widget_)
      parent_widget_->update();
    else if (info->applet)
      info->applet->update();
  }

  void QueueResize() {
    if (info->applet)
      info->applet->update();
  }

  bool ShowContextMenu(int button) {
    ASSERT(view_);
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
  Connection *onoptionchanged_connection_;

  Slot1<void, int> *feedback_handler_;
  QString caption_;
  QMenu context_menu_;

  void Detach() {
    view_ = NULL;
  }

  void HandleOptionViewResponse(ViewInterface::OptionsViewFlags flag) {
    if (feedback_handler_) {
      (*feedback_handler_)(flag);
      delete feedback_handler_;
      feedback_handler_ = NULL;
    }
    parent_widget_->hide();
  }


 public slots:
  void OnViewMoved(int x, int y);
  void OnOptionViewOK();
  void OnOptionViewCancel();
};

} // namespace ggadget

#endif
