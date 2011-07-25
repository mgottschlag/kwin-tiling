/*
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "outputconfig.h"
#include "outputgraphicsitem.h"
#include "collapsiblewidget.h"
#include "layoutmanager.h"

#include "randrconfig.h"
#include "randroutput.h"
#include "randrdisplay.h"
#include "randrscreen.h"

#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kshell.h>
#include <qmenu.h>

RandRConfig::RandRConfig(QWidget *parent, RandRDisplay *display)
	: QWidget(parent), Ui::RandRConfigBase()
{
	m_display = display;
	Q_ASSERT(m_display);
	
	m_changed = false;

	if (!m_display->isValid()) {
		// FIXME: this needs much better handling of this error...
		return;
	}

	setupUi(this);
	layout()->setMargin(0);

	connect( identifyOutputsButton, SIGNAL( clicked()), SLOT( identifyOutputs()));
	connect( &identifyTimer, SIGNAL( timeout()), SLOT( clearIndicators()));
	connect( &compressUpdateViewTimer, SIGNAL( timeout()), SLOT( slotDelayedUpdateView()));
	connect(unifyOutputs, SIGNAL(toggled(bool)), SLOT(unifiedOutputChanged(bool)));

	identifyTimer.setSingleShot( true );
	compressUpdateViewTimer.setSingleShot( true );

	connect( saveAsDefaultButton, SIGNAL( clicked()), SLOT( saveStartup()));
	QMenu* saveMenu = new QMenu(saveAsDefaultButton);
	saveMenu->addAction(i18n("Save as Default"),this, SLOT(saveStartup()));
	saveMenu->addAction(i18n("Reset"),this, SLOT(disableStartup()));
	saveAsDefaultButton->setMenu(saveMenu);

	// create the container for the settings widget
	QHBoxLayout *layout = new QHBoxLayout(outputList);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	m_container = new SettingsContainer(outputList);
	m_container->setSizePolicy(QSizePolicy::Minimum,
						  QSizePolicy::Minimum);
	layout->addWidget(m_container);

#ifdef HAS_RANDR_1_3
        if (RandR::has_1_3)
        {
            primaryDisplayBox->setVisible(true);
            label->setVisible(true);
        }
        else
#endif //HAS_RANDR_1_3
        {
            primaryDisplayBox->setVisible(false);
            label->setVisible(false);
        }

	KConfig config("krandrrc");
	if (config.hasGroup("Screen_0") && config.group("Screen_0").readEntry("OutputsUnified", false)) {
		unifyOutputs->setChecked(true);
	}
	// create the scene
	m_scene = new QGraphicsScene(m_display->currentScreen()->rect());	
	screenView->setScene(m_scene);
	screenView->installEventFilter(this);

	m_layoutManager = new LayoutManager(m_display->currentScreen(), m_scene);
}

RandRConfig::~RandRConfig()
{
	clearIndicators();
	delete m_scene;
}

void RandRConfig::load(void)
{
	if (!m_display->isValid()) {
		kDebug() << "Invalid display! Aborting config load.";
		return;
	}

	m_scene->clear();
	qDeleteAll(m_outputList);
	m_outputList.clear();
	m_configs.clear(); // objects deleted above
	
	OutputMap outputs = m_display->currentScreen()->outputs();
#ifdef HAS_RANDR_1_3
	RandROutput *primary = m_display->currentScreen()->primaryOutput();
	if (RandR::has_1_3)
	{
		// disconnect while we repopulate the combo box
		disconnect(primaryDisplayBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChanged()));
		primaryDisplayBox->clear();
		primaryDisplayBox->addItem(i18nc("No display selected", "None"));
	}
#endif //HAS_RANDR_1_3

	// FIXME: adjust it to run on a multi screen system
	CollapsibleWidget *w;
	OutputGraphicsItem *o;
	OutputConfigList preceding;
	foreach(RandROutput *output, outputs)
	{
		OutputConfig *config = new OutputConfig(this, output, preceding, unifyOutputs->isChecked());
		m_configs.append( config );
		preceding.append( config );
		
		QString description = output->isConnected()
			? i18n("%1 (Connected)", output->name())
			: output->name();
		w = m_container->insertWidget(config, description);
		if(output->isConnected()) {
			w->setExpanded(true);
			kDebug() << "Output rect:" << output->rect();
		}
		connect(config, SIGNAL(connectedChanged(bool)), this, SLOT(outputConnectedChanged(bool)));
		m_outputList.append(w);
		
		o = new OutputGraphicsItem(config);
		m_scene->addItem(o);
		
		connect(o,    SIGNAL(itemChanged(OutputGraphicsItem*)), 
		        this, SLOT(slotAdjustOutput(OutputGraphicsItem*)));

		connect(config, SIGNAL(updateView()), this, SLOT(slotUpdateView()));
		connect(config, SIGNAL(optionChanged()), this, SLOT(slotChanged()));

#ifdef HAS_RANDR_1_3
		if (RandR::has_1_3 && output->isConnected())
		{
			primaryDisplayBox->addItem(output->name(), QVariant::fromValue(output->id()));
			if (primary == output)
			{
				primaryDisplayBox->setCurrentIndex(primaryDisplayBox->count()-1);
			}
		}
#endif //HAS_RANDR_1_3
	}
#ifdef HAS_RANDR_1_3
	if (RandR::has_1_3)
	{
		connect(primaryDisplayBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChanged()));
	}
#endif //HAS_RANDR_1_3
	slotUpdateView();
}

void RandRConfig::outputConnectedChanged(bool connected)
{
	OutputConfig *config = static_cast <OutputConfig *> (sender());
	int index = m_configs.indexOf(config);
	QString description = connected
			? i18n("%1 (Connected)", config->output()->name())
			: config->output()->name();
	m_outputList.at(index)->setCaption(description);
}

void RandRConfig::save()
{
	if (!m_display->isValid())
		return;

	KConfig config("krandrrc");
	if (config.hasGroup("Screen_0")) {
		config.group("Screen_0").writeEntry("OutputsUnified", unifyOutputs->isChecked());
		config.sync();
	}
	apply();
}

void RandRConfig::defaults()
{
	update();
}

void RandRConfig::apply()
{
	kDebug() << "Applying settings...";

	// normalize positions so that the coordinate system starts at (0,0)
	QPoint normalizePos;
	bool first = true;
	foreach(CollapsibleWidget *w, m_outputList) {
		OutputConfig *config = static_cast<OutputConfig *>(w->innerWidget());
		if( config->isActive()) {
			QPoint pos = config->position();
			if( first ) {
				normalizePos = pos;
				first = false;
			} else {
				if( pos.x() < normalizePos.x())
					normalizePos.setX( pos.x());
				if( pos.y() < normalizePos.y())
					normalizePos.setY( pos.y());
			}
		}
	}
	normalizePos = -normalizePos;
	kDebug() << "Normalizing positions by" << normalizePos;

	foreach(CollapsibleWidget *w, m_outputList) {
		OutputConfig *config = static_cast<OutputConfig *>(w->innerWidget());
		RandROutput *output = config->output();
		
		if(!output->isConnected())
			continue;
		
		QSize res = config->resolution();
		
		if(!res.isNull()) {
			if(!config->hasPendingChanges( normalizePos )) {
				kDebug() << "Ignoring identical config for" << output->name();
				continue;
			}
			QRect configuredRect(config->position(), res);
			
			kDebug() << "Output config for" << output->name() << ":\n"
			            "  rect =" << configuredRect
			         << ", rot =" << config->rotation()
			         << ", rate =" << config->refreshRate();
			
			// Break the connection with the previous CRTC for changed outputs, since
			// otherwise the code could try to use the same CRTC for two different outputs.
			// This is probably rather hackish and may not always work, but I don't see
			// a better way with this codebase, definitely not with the time I have now.
			output->disconnectFromCrtc();

			output->proposeRect(configuredRect.translated( normalizePos ));
			output->proposeRotation(config->rotation());
			output->proposeRefreshRate(config->refreshRate());
		} else { // user wants to disable this output
			kDebug() << "Disabling" << output->name();
			output->slotDisable();
		}
	}
#ifdef HAS_RANDR_1_3
	if (RandR::has_1_3)
	{
		int primaryOutputIndex = primaryDisplayBox->currentIndex();
		RandRScreen *screen = m_display->currentScreen();
		if (primaryOutputIndex > 0)
		{
			QVariant output = primaryDisplayBox->itemData(primaryOutputIndex);
			screen->proposePrimaryOutput(screen->output(output.value<RROutput>()));
		}
		else
		{
			screen->proposePrimaryOutput(0);
		}
	}
#endif //HAS_RANDR_1_3
	m_display->applyProposed();
	update();
}

void RandRConfig::slotChanged(void)
{
	m_changed = true;
	
	emit changed(true);
}

void RandRConfig::update()
{
	// TODO: implement
	m_changed = false;
	emit changed(false);
}

void RandRConfig::saveStartup()
{
	if (!m_display->isValid())
		return;
	KConfig config("krandrrc");
	m_display->saveStartup(config);
#ifdef HAS_RANDR_1_3
	if (RandR::has_1_3)
	{
		// Add setting the primary screen to the list of commands
		KConfigGroup group = config.group("Display");
		QStringList commands = group.readEntry("StartupCommands").split("\n");
		int primaryOutputIndex = primaryDisplayBox->currentIndex();
		if (primaryOutputIndex > 0)
		{
			QString primaryOutput = primaryDisplayBox->itemText(primaryOutputIndex);
			commands += QString("xrandr --output %1 --primary")
			    .arg( KShell::quoteArg( primaryOutput ));
		}
		else
			commands += "xrandr --noprimary";
		group.writeEntry("StartupCommands",commands.join("\n"));
	}
#endif //HAS_RANDR_1_3
	KMessageBox::information( window(), i18n( "Configuration has been set as the desktop default." ));
}

void RandRConfig::disableStartup()
{
	if (!m_display->isValid())
		return;
	KConfig config("krandrrc");
	m_display->disableStartup(config);
	KMessageBox::information( window(), i18n( "Default desktop setup has been reset." ));
}

void RandRConfig::unifiedOutputChanged(bool checked)
{
	Q_FOREACH(OutputConfig *config, m_configs) {
		config->setUnifyOutput(checked);
		config->updateSizeList();
	}
	slotChanged();
}

bool RandRConfig::eventFilter(QObject *obj, QEvent *event)
{
	if ( obj == screenView && event->type() == QEvent::Resize ) {
		slotUpdateView();
		return false;
	} else {
		return QWidget::eventFilter(obj, event);
	}
}

void RandRConfig::slotAdjustOutput(OutputGraphicsItem *o)
{
    Q_UNUSED(o);
	kDebug() << "Output graphics item changed:";
	
	// TODO: Implement
}

void RandRConfig::slotUpdateView()
{
	compressUpdateViewTimer.start( 0 );
}

#include <typeinfo>

void RandRConfig::slotDelayedUpdateView()
{
	QRect r;
	bool first = true;

	// updates the graphics view so that all outputs fit inside of it
	foreach(OutputConfig *config, m_configs)
	{		
		if (first)
		{
			first = false;
			r = config->rect();
		}
		else
			r = r.united(config->rect());
	}
	// scale the total bounding rectangle for all outputs to fit
	// 80% of the containing QGraphicsView
	float scaleX = (float)screenView->width() / r.width();
	float scaleY = (float)screenView->height() / r.height();
	float scale = (scaleX < scaleY) ? scaleX : scaleY;
	scale *= 0.80f;
	
	screenView->resetMatrix();
	screenView->scale(scale,scale);
	screenView->ensureVisible(r);
	screenView->setSceneRect(r);

	foreach( QGraphicsItem* item, m_scene->items()) {
		if( OutputGraphicsItem* itemo = dynamic_cast< OutputGraphicsItem* >( item ))
			itemo->configUpdated();
	}
	screenView->update();
}

uint qHash( const QPoint& p )
{
	return p.x() * 10000 + p.y();
}

void RandRConfig::identifyOutputs()
{
	identifyTimer.stop();
	clearIndicators();
	QHash< QPoint, QStringList > ids; // outputs at centers of screens (can be more in case of clone mode)
	OutputMap outputs = m_display->currentScreen()->outputs();
	foreach(RandROutput *output, outputs)
	{
		if( !output->isConnected() || output->rect().isEmpty())
			continue;
		ids[ output->rect().center() ].append( output->name());
	}
	for( QHash< QPoint, QStringList >::ConstIterator it = ids.constBegin();
	     it != ids.constEnd();
	     ++it )
	{
		QLabel *si = new QLabel(it->join("\n"), NULL, Qt::X11BypassWindowManagerHint);
		QFont fnt = KGlobalSettings::generalFont();
		fnt.setPixelSize(100);
		si->setFont(fnt);
		si->setFrameStyle(QFrame::Panel);
		si->setFrameShadow(QFrame::Plain);
		si->setAlignment(Qt::AlignCenter);
		QRect targetGeometry(QPoint(0,0), si->sizeHint());
	        targetGeometry.moveCenter(it.key());
		si->setGeometry(targetGeometry);
		si->show();
	        m_indicators.append( si );
	}
	identifyTimer.start( 1500 );
}

void RandRConfig::clearIndicators()
{
	qDeleteAll( m_indicators );
	m_indicators.clear();
}

void RandRConfig::insufficientVirtualSize()
{
    if( KMessageBox::questionYesNo( this,
        i18n( "Insufficient virtual size for the total screen size.\n"
              "The configured virtual size of your X server is insufficient for this setup. "
              "This configuration needs to be adjusted.\n"
              "Do you wish to run a tool to adjust the configuration?" )) == KMessageBox::Yes )
        {
        KProcess proc;
        // TODO
        if( proc.execute() == 0 )
            KMessageBox::information( this, i18n( "Configuration has been adjusted. Please restart "
                "your session for this change to take effect." ));
        else
            KMessageBox::sorry( this, i18n( "Changing configuration failed. Please adjust your xorg.conf manually." ));
        }
}

bool RandRConfig::x11Event(XEvent* e)
{
    kDebug() << "PAPAPAPA";
    return QWidget::x11Event(e);
}

#include "randrconfig.moc"

// vim:noet:sts=8:sw=8:
