/*

 This is original Qt QX11Embed* code, with some modifications (check history,
 the first commit was the original unmodified code) to allow embedding window
 with different visual/depths/whatever. QX11EmbedContainer should handle this
 on its own, so this should be eventually done a bit more properly and
 submitted to TT.

*/

/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QX11EMBED_X11_H
#define QX11EMBED_X11_H

#include <QtGui/qwidget.h>

namespace PlasmaSystray
{

class QX11EmbedWidgetPrivate;
class Q_GUI_EXPORT QX11EmbedWidget : public QWidget
{
    Q_OBJECT
public:
    QX11EmbedWidget(QWidget *parent = 0);
    ~QX11EmbedWidget();

    void embedInto(WId id);
    WId containerWinId() const;

    enum Error {
	Unknown,
	Internal,
	InvalidWindowID
    };
    Error error() const;

Q_SIGNALS:
    void embedded();
    void containerClosed();
    void error(QX11EmbedWidget::Error error);

protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);

private:
    Q_DECLARE_PRIVATE(QX11EmbedWidget)
    Q_DISABLE_COPY(QX11EmbedWidget)
};

class QX11EmbedContainerPrivate;
class Q_GUI_EXPORT QX11EmbedContainer : public QWidget
{
    Q_OBJECT
public:
    QX11EmbedContainer(WId prepareid, QWidget *parent = 0);
    ~QX11EmbedContainer();

    void embedClient(WId id);
    void discardClient();

    WId clientWinId() const;

    QSize minimumSizeHint() const;

    enum Error {
	Unknown,
	Internal,
	InvalidWindowID
    };
    Error error() const;

Q_SIGNALS:
    void clientIsEmbedded();
    void clientClosed();
    void error(QX11EmbedContainer::Error);

protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    bool event(QEvent *);

private:
    Q_DECLARE_PRIVATE(QX11EmbedContainer)
    Q_DISABLE_COPY(QX11EmbedContainer)
};

} // namespace

typedef PlasmaSystray::QX11EmbedContainer KX11EmbedContainer;

#endif // QX11EMBED_X11_H
