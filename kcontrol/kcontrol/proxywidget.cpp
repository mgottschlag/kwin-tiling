/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <unistd.h> // for getuid()

#include <kpushbutton.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QByteArray>
#include <Q3Frame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <klocale.h>
#include <kapplication.h>
#include <kcmodule.h>
#include <kseparator.h>
#include <kdialog.h>
#include <kstdguiitem.h>
#include <dcopclient.h>


#include <qlabel.h>

#include "global.h"
#include "proxywidget.h"
#include "proxywidget.moc"
#include <Q3WhatsThis>
#include <kdebug.h>
#include <qtimer.h>

class WhatsThis : public Q3WhatsThis
{
public:
    WhatsThis( ProxyWidget* parent )
    : Q3WhatsThis( parent ), proxy( parent ) {}
    ~WhatsThis(){};


    QString text( const QPoint &  ) {
    if ( !proxy->quickHelp().isEmpty() )
        return proxy->quickHelp();
    else
        return i18n("The currently loaded configuration module.");
    }

private:
    ProxyWidget* proxy;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setVisible(QPushButton *btn, bool vis)
{
  if (vis)
    btn->show();
  else
    btn->hide();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////


class RootInfoWidget : public QLabel
{
public:
    RootInfoWidget(QWidget *parent, const char *name);
    void setRootMsg(const QString& s) { setText(s); }
};

RootInfoWidget::RootInfoWidget(QWidget *parent, const char *name = 0)
    : QLabel(parent, name)
{
    setFrameShape(Q3Frame::Box);
    setFrameShadow(Q3Frame::Raised);

    setText(i18n("<b>Changes in this module require root access.</b><br>"
                      "Click the \"Administrator Mode\" button to "
                      "allow modifications in this module."));

	this->setWhatsThis( i18n("This module requires special permissions, probably "
                              "for system-wide modifications; therefore, it is "
                              "required that you provide the root password to be "
                              "able to change the module's properties.  If you "
                              "do not provide the password, the module will be "
                              "disabled."));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

class ProxyView : public Q3ScrollView
{
public:
    ProxyView(KCModule *client, const QString& title, QWidget *parent, bool run_as_root, const char *name);

private:
    virtual void resizeEvent(QResizeEvent *);

    QWidget *contentWidget;
    KCModule    *client;
    bool scroll;
};

class ProxyContentWidget : public QWidget
{
public:
    ProxyContentWidget( QWidget* parent ) : QWidget( parent ) {}
    ~ProxyContentWidget(){}

    // this should be really done by qscrollview in AutoOneFit mode!
    QSize sizeHint() const { return minimumSizeHint(); }
};


ProxyView::ProxyView(KCModule *_client, const QString&, QWidget *parent, bool run_as_root, const char *name)
    : Q3ScrollView(parent, name), client(_client)
{
  setResizePolicy(Q3ScrollView::AutoOneFit);
  setFrameStyle( NoFrame );
  contentWidget = new ProxyContentWidget( viewport() );

  QVBoxLayout* vbox = new QVBoxLayout( contentWidget );

  if (run_as_root && _client->useRootOnlyMsg()) // notify the user
  {
      RootInfoWidget *infoBox = new RootInfoWidget(contentWidget);
      vbox->addWidget( infoBox );
      QString msg = _client->rootOnlyMsg();
      if (!msg.isEmpty())
	      infoBox->setRootMsg(msg);
      vbox->setSpacing(KDialog::spacingHint());
  }
  client->reparent(contentWidget,0,QPoint(0,0),true);
  vbox->addWidget( client );
  vbox->activate(); // make sure we have a proper minimumSizeHint
  addChild(contentWidget);
}

void ProxyView::resizeEvent(QResizeEvent *e)
{
    Q3ScrollView::resizeEvent(e);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////


ProxyWidget::ProxyWidget(KCModule *client, QString title, const char *name,
             bool run_as_root)
  : QWidget(0, name)
  , _client(client)
{
 setCaption(title);

 if (getuid()==0 ) {
	 // Make root modules look as similar as possible...
	 DCOPCString replyType;
	 QByteArray replyData;
	 
	 if (kapp->dcopClient()->call("kcontrol", "moduleIface", "getPalette()", QByteArray(),
				 replyType, replyData))
		 if ( replyType == "QPalette") {
			 QDataStream reply( replyData );
			 QPalette pal;
			 reply >> pal;
			 setPalette(pal);
		 }
/* // Doesn't work ...
	 if (kapp->dcopClient()->call("kcontrol", "moduleIface", "getStyle()", QByteArray(),
				 replyType, replyData))
		 if ( replyType == "QString") {
			 QDataStream reply( replyData, IO_ReadOnly );
			 QString style; 
			 reply >> style;
			 setStyle(style);
		 }
*/	 
	 if (kapp->dcopClient()->call("kcontrol", "moduleIface", "getFont()", QByteArray(),
				 replyType, replyData))
		 if ( replyType == "QFont") {
			 QDataStream reply( replyData );
			 QFont font;
			 reply >> font;
			 setFont(font);
		 }
 }
	 
  view = new ProxyView(client, title, this, run_as_root, "proxyview");
  (void) new WhatsThis( this );

  connect(_client, SIGNAL(changed(bool)), SLOT(clientChanged(bool)));
  connect(_client, SIGNAL(quickHelpChanged()), SIGNAL(quickHelpChanged()));

  _sep = new KSeparator(Qt::Horizontal, this);

  _help =    new KPushButton( KStdGuiItem::help(), this );
  _default = new KPushButton( KStdGuiItem::defaults(), this );
  _apply =   new KPushButton( KStdGuiItem::apply(), this );
  _reset =   new KPushButton( KGuiItem( i18n( "&Reset" ), "undo" ), this );
  _root =    new KPushButton( KGuiItem(i18n( "&Administrator Mode" )), this );

  bool mayModify = (!run_as_root || !_client->useRootOnlyMsg()) && !KCGlobal::isInfoCenter();

  // only enable the requested buttons
  int b = _client->buttons();
  _help->setVisible(false & (b & KCModule::Help));
  _default->setVisible(mayModify && (b & KCModule::Default));
  _apply->setVisible(mayModify && (b & KCModule::Apply));
  _reset->setVisible(mayModify && (b & KCModule::Apply));
  _root->setVisible(run_as_root);

  // disable initial buttons
  _apply->setEnabled( false );
  _reset->setEnabled( false );

  connect(_help, SIGNAL(clicked()), SLOT(helpClicked()));
  connect(_default, SIGNAL(clicked()), SLOT(defaultClicked()));
  connect(_apply, SIGNAL(clicked()), SLOT(applyClicked()));
  connect(_reset, SIGNAL(clicked()), SLOT(resetClicked()));
  connect(_root, SIGNAL(clicked()), SLOT(rootClicked()));

  QVBoxLayout *top = new QVBoxLayout(this, KDialog::marginHint(), 
      KDialog::spacingHint());
  top->addWidget(view);
  top->addWidget(_sep);

  QHBoxLayout *buttons = new QHBoxLayout(top, 4);
  buttons->addWidget(_help);
  buttons->addWidget(_default);
  if (run_as_root) 
  {
    buttons->addWidget(_root);
  }

  buttons->addStretch(1);
  if (mayModify)
  {
    buttons->addWidget(_apply);
    buttons->addWidget(_reset);
  }

  top->activate();
}

ProxyWidget::~ProxyWidget()
{
  delete _client;
}

QString ProxyWidget::quickHelp() const
{
  if (_client)
    return _client->quickHelp();
  else
    return "";
}

void ProxyWidget::helpClicked()
{
  if (getuid()!=0)
	  emit helpRequest();
  else
     kapp->dcopClient()->send("kcontrol", "moduleIface", "invokeHelp()", QByteArray());
}

void ProxyWidget::defaultClicked()
{
  clientChanged(true);
  _client->defaults();
}

void ProxyWidget::applyClicked()
{
  _client->save();
  clientChanged(false);
}

void ProxyWidget::resetClicked()
{
  _client->load();
  clientChanged(false);
}

void ProxyWidget::rootClicked()
{
  emit runAsRoot();
}

void ProxyWidget::clientChanged(bool state)
{
  _apply->setEnabled(state);
  _reset->setEnabled(state);

  // forward the signal
  emit changed(state);
}

const KAboutData *ProxyWidget::aboutData() const
{
  return _client->aboutData();
}

// vim: sw=2 sts=2 et
