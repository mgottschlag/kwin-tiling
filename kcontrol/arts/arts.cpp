/*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

*/

#include <unistd.h>

#include <qcombobox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qslider.h>
#include <qtabwidget.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QByteArray>

#include <dcopref.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmoduleloader.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krichtextlabel.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <libkmid/deviceman.h>

#include "arts.h"

extern "C" {
	KDE_EXPORT void init_arts();

    KDE_EXPORT KCModule *create_arts(QWidget *parent, const char* /*name*/)
	{
		KGlobal::locale()->insertCatalogue("kcmarts");
		return new KArtsModule(parent, "kcmarts" );
	}
}

static bool startArts()
{
	KConfig *config = new KConfig("kcmartsrc", true, false);

	config->setGroup("Arts");
	bool startServer = config->readBoolEntry("StartServer",true);
	bool startRealtime = config->readBoolEntry("StartRealtime",true);
	QString args = config->readEntry("Arguments","-F 10 -S 4096 -s 60 -m artsmessage -c drkonqi -l 3 -f");

	delete config;

	if (startServer)
		kapp->kdeinitExec(startRealtime?"artswrapper":"artsd",
		                  QStringList::split(" ",args));
	return startServer;
}

/*
 * This function uses artsd -A to init audioIOList with the possible audioIO
 * methods. Here is a sample output of artsd -A (note the two spaces before
 * each "interesting" line are used in parsing:
 *
 * # artsd -A
 * possible choices for the audio i/o method:
 *
 *   toss      Threaded Open Sound System
 *   esd       Enlightened Sound Daemon
 *   null      No audio input/output
 *   alsa      Advanced Linux Sound Architecture
 *   oss       Open Sound System
 *
 */
void KArtsModule::initAudioIOList()
{
	KProcess* artsd = new KProcess();
	*artsd << "artsd";
	*artsd << "-A";

	connect(artsd, SIGNAL(processExited(KProcess*)),
	        this, SLOT(slotArtsdExited(KProcess*)));
	connect(artsd, SIGNAL(receivedStderr(KProcess*, char*, int)),
	        this, SLOT(slotProcessArtsdOutput(KProcess*, char*, int)));

	if (!artsd->start(KProcess::Block, KProcess::Stderr)) {
		KMessageBox::error(0, i18n("Unable to start the sound server to "
		                           "retrieve possible sound I/O methods.\n"
		                           "Only automatic detection will be "
		                           "available."));
                delete artsd;
	}
}

void KArtsModule::slotArtsdExited(KProcess* proc)
{
	latestProcessStatus = proc->exitStatus();
	delete proc;
}

void KArtsModule::slotProcessArtsdOutput(KProcess*, char* buf, int len)
{
	// XXX(gioele): I suppose this will be called with full lines, am I wrong?

	QStringList availableIOs = QStringList::split("\n", QByteArray(buf, len));
	// valid entries have two leading spaces
	availableIOs = availableIOs.grep(QRegExp("^ {2}"));
	availableIOs.sort();

	QString name, fullName;
	QStringList::Iterator it;
	for (it = availableIOs.begin(); it != availableIOs.end(); ++it) {
		name = (*it).left(12).stripWhiteSpace();
		fullName = (*it).mid(12).stripWhiteSpace();
		audioIOList.append(new AudioIOElement(name, fullName));
	}
}

KArtsModule::KArtsModule(QWidget *parent, const char *name)
  : KCModule(parent, name), configChanged(false)
{
	setButtons(Default|Apply);

	setQuickHelp( i18n("<h1>Sound System</h1> Here you can configure aRts, KDE's sound server."
		    " This program not only allows you to hear your system sounds while simultaneously"
		    " listening to an MP3 file or playing a game with background music. It also allows you"
		    " to apply different effects to your system sounds and provides programmers with"
		    " an easy way to achieve sound support."));

	initAudioIOList();

	QVBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint());
	QTabWidget *tab = new QTabWidget(this);
	layout->addWidget(tab);

	general = new generalTab(tab);
	hardware = new hardwareTab(tab);
	//mixer = KCModuleLoader::loadModule("kmixcfg", tab);
	//midi = new KMidConfig(tab, "kmidconfig");

	general->layout()->setMargin( KDialog::marginHint() );
	hardware->layout()->setMargin( KDialog::marginHint() );
	general->latencyLabel->setFixedHeight(general->latencyLabel->fontMetrics().lineSpacing());

	tab->addTab(general, i18n("&General"));
	tab->addTab(hardware, i18n("&Hardware"));

	startServer = general->startServer;
	networkTransparent = general->networkTransparent;
	startRealtime = general->startRealtime;
	autoSuspend = general->autoSuspend;
	suspendTime = general->suspendTime;

	fullDuplex = hardware->fullDuplex;
	customDevice = hardware->customDevice;
	deviceName = hardware->deviceName;
	customRate = hardware->customRate;
	samplingRate = hardware->samplingRate;

   	QString deviceHint = i18n("Normally, the sound server defaults to using the device called <b>/dev/dsp</b> for sound output. That should work in most cases. On some systems where devfs is used, however, you may need to use <b>/dev/sound/dsp</b> instead. Other alternatives are things like <b>/dev/dsp0</b> or <b>/dev/dsp1</b>, if you have a soundcard that supports multiple outputs, or you have multiple soundcards.");

	QString rateHint = i18n("Normally, the sound server defaults to using a sampling rate of 44100 Hz (CD quality), which is supported on almost any hardware. If you are using certain <b>Yamaha soundcards</b>, you might need to configure this to 48000 Hz here, if you are using <b>old SoundBlaster cards</b>, like SoundBlaster Pro, you might need to change this to 22050 Hz. All other values are possible, too, and may make sense in certain contexts (i.e. professional studio equipment).");

	QString optionsHint = i18n("This configuration module is intended to cover almost every aspect of the aRts sound server that you can configure. However, there are some things which may not be available here, so you can add <b>command line options</b> here which will be passed directly to <b>artsd</b>. The command line options will override the choices made in the GUI. To see the possible choices, open a Konsole window, and type <b>artsd -h</b>.");

	customDevice->setWhatsThis( deviceHint);
	deviceName->setWhatsThis( deviceHint);
	customRate->setWhatsThis( rateHint);
	samplingRate->setWhatsThis( rateHint);
	hardware->customOptions->setWhatsThis( optionsHint);
	hardware->addOptions->setWhatsThis( optionsHint);

	hardware->audioIO->insertItem( i18n( "Autodetect" ) );
	for (AudioIOElement *a = audioIOList.first(); a != 0; a = audioIOList.next())
		hardware->audioIO->insertItem(i18n(a->fullName.utf8()));

	deviceManager = new DeviceManager();
	deviceManager->initManager();

	QString s;
	for ( int i = 0; i < deviceManager->midiPorts()+deviceManager->synthDevices(); i++)
	{
		if ( strcmp( deviceManager->type( i ), "" ) != 0 )
			s.sprintf( "%s - %s", deviceManager->name( i ), deviceManager->type( i ) );
		else
			s.sprintf( "%s", deviceManager->name( i ) );

		hardware->midiDevice->insertItem( s, i );

	};

	config = new KConfig("kcmartsrc");
	GetSettings();

	suspendTime->setRange( 0, 999, 1, true );

	connect(startServer,SIGNAL(clicked()),this,SLOT(slotChanged()));
	connect(networkTransparent,SIGNAL(clicked()),this,SLOT(slotChanged()));
	connect(startRealtime,SIGNAL(clicked()),this,SLOT(slotChanged()));
	connect(fullDuplex,SIGNAL(clicked()),this,SLOT(slotChanged()));
	connect(customDevice, SIGNAL(clicked()), SLOT(slotChanged()));
	connect(deviceName, SIGNAL(textChanged(const QString&)), SLOT(slotChanged()));
	connect(customRate, SIGNAL(clicked()), SLOT(slotChanged()));
	connect(samplingRate, SIGNAL(valueChanged(const QString&)), SLOT(slotChanged()));
//	connect(general->volumeSystray, SIGNAL(clicked()), this, SLOT(slotChanged()) );

	connect(hardware->audioIO,SIGNAL(highlighted(int)),SLOT(slotChanged()));
	connect(hardware->audioIO,SIGNAL(activated(int)),SLOT(slotChanged()));
	connect(hardware->customOptions,SIGNAL(clicked()),SLOT(slotChanged()));
	connect(hardware->addOptions,SIGNAL(textChanged(const QString&)),SLOT(slotChanged()));
	connect(hardware->soundQuality,SIGNAL(highlighted(int)),SLOT(slotChanged()));
	connect(hardware->soundQuality,SIGNAL(activated(int)),SLOT(slotChanged()));
	connect(general->latencySlider,SIGNAL(valueChanged(int)),SLOT(slotChanged()));
	connect(autoSuspend,SIGNAL(clicked()),SLOT(slotChanged()));
	connect(suspendTime,SIGNAL(valueChanged(int)),SLOT(slotChanged()));
	connect(general->testSound,SIGNAL(clicked()),SLOT(slotTestSound()));
	connect(general->testMIDI,SIGNAL(clicked()),SLOT(slotTestMIDI()));
	connect(hardware->midiDevice, SIGNAL( highlighted(int) ), this, SLOT( slotChanged() ) );
	connect(hardware->midiDevice, SIGNAL( activated(int) ), this, SLOT( slotChanged() ) );
	connect(hardware->midiUseMapper, SIGNAL( clicked() ), this, SLOT( slotChanged() ) );
	connect(hardware->midiMapper, SIGNAL( textChanged( const QString& ) ),
			this, SLOT( slotChanged() ) );

	KAboutData *about =  new KAboutData(I18N_NOOP("kcmarts"),
                  I18N_NOOP("The Sound Server Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1999 - 2001, Stefan Westerfeld"));
	about->addAuthor("Stefan Westerfeld",I18N_NOOP("aRts Author") , "stw@kde.org");
	setAboutData(about);
}

void KArtsModule::GetSettings( void )
{
	config->setGroup("Arts");
	startServer->setChecked(config->readBoolEntry("StartServer",true));
	startRealtime->setChecked(config->readBoolEntry("StartRealtime",true) &&
	                          realtimeIsPossible());
	networkTransparent->setChecked(config->readBoolEntry("NetworkTransparent",false));
	fullDuplex->setChecked(config->readBoolEntry("FullDuplex",false));
	autoSuspend->setChecked(config->readBoolEntry("AutoSuspend",true));
	suspendTime->setValue(config->readNumEntry("SuspendTime",60));
	deviceName->setText(config->readEntry("DeviceName",QString::null));
	customDevice->setChecked(!deviceName->text().isEmpty());
	hardware->addOptions->setText(config->readEntry("AddOptions",QString::null));
	hardware->customOptions->setChecked(!hardware->addOptions->text().isEmpty());
	general->latencySlider->setValue(config->readNumEntry("Latency",250));

	int rate = config->readNumEntry("SamplingRate",0);
	if(rate)
	{
		customRate->setChecked(true);
		samplingRate->setValue(rate);
	}
	else
	{
		customRate->setChecked(false);
		samplingRate->setValue(44100);
	}

	switch (config->readNumEntry("Bits", 0)) {
	case 0:
		hardware->soundQuality->setCurrentItem(0);
		break;
	case 16:
		hardware->soundQuality->setCurrentItem(1);
		break;
	case 8:
		hardware->soundQuality->setCurrentItem(2);
		break;
	}

	QString audioIO = config->readEntry("AudioIO", QString::null);
	hardware->audioIO->setCurrentItem(0);
	for(AudioIOElement *a = audioIOList.first(); a != 0; a = audioIOList.next())
	{
		if(a->name == audioIO)		// first item: "autodetect"
		  {
			hardware->audioIO->setCurrentItem(audioIOList.at() + 1);
			break;
		  }

	}

//	config->setGroup( "Mixer" );
//	general->volumeSystray->setChecked( config->readBoolEntry( "VolumeControlOnSystray", true ) );

	KConfig *midiConfig = new KConfig( "kcmmidirc", true );

	midiConfig->setGroup( "Configuration" );
	hardware->midiDevice->setCurrentItem( midiConfig->readNumEntry( "midiDevice", 0 ) );
	QString mapurl( midiConfig->readPathEntry( "mapFilename" ) );
	hardware->midiMapper->setURL( mapurl );
	hardware->midiUseMapper->setChecked( midiConfig->readBoolEntry( "useMidiMapper", false ) );
	hardware->midiMapper->setEnabled( hardware->midiUseMapper->isChecked() );

	delete midiConfig;

	updateWidgets();
	emit changed( false );
}

KArtsModule::~KArtsModule() {
        delete config;
        audioIOList.setAutoDelete(true);
        audioIOList.clear();
}

void KArtsModule::saveParams( void )
{
	QString audioIO;

	int item = hardware->audioIO->currentItem() - 1;	// first item: "default"

	if (item >= 0) {
		audioIO = audioIOList.at(item)->name;
	}

	QString dev = customDevice->isChecked() ? deviceName->text() : QString::null;
	int rate = customRate->isChecked()?samplingRate->value() : 0;
	QString addOptions;
	if(hardware->customOptions->isChecked())
		addOptions = hardware->addOptions->text();

	int latency = general->latencySlider->value();
	int bits = 0;

	if (hardware->soundQuality->currentItem() == 1)
		bits = 16;
	else if (hardware->soundQuality->currentItem() == 2)
		bits = 8;

	config->setGroup("Arts");
	config->writeEntry("StartServer",startServer->isChecked());
	config->writeEntry("StartRealtime",startRealtime->isChecked());
	config->writeEntry("NetworkTransparent",networkTransparent->isChecked());
	config->writeEntry("FullDuplex",fullDuplex->isChecked());
	config->writeEntry("DeviceName",dev);
	config->writeEntry("SamplingRate",rate);
	config->writeEntry("AudioIO",audioIO);
	config->writeEntry("AddOptions",addOptions);
	config->writeEntry("Latency",latency);
	config->writeEntry("Bits",bits);
	config->writeEntry("AutoSuspend", autoSuspend->isChecked());
	config->writeEntry("SuspendTime", suspendTime->value());
	calculateLatency();
	// Save arguments string in case any other process wants to restart artsd.

	config->writeEntry("Arguments",
		createArgs(networkTransparent->isChecked(), fullDuplex->isChecked(),
					fragmentCount, fragmentSize, dev, rate, bits,
					audioIO, addOptions, autoSuspend->isChecked(),
					suspendTime->value() ));

//	config->setGroup( "Mixer" );
//	config->writeEntry( "VolumeControlOnSystray", general->volumeSystray->isChecked() );

	KConfig *midiConfig = new KConfig( "kcmmidirc", false );

	midiConfig->setGroup( "Configuration" );
	midiConfig->writeEntry( "midiDevice", hardware->midiDevice->currentItem() );
	midiConfig->writeEntry( "useMidiMapper", hardware->midiUseMapper->isChecked() );
	midiConfig->writePathEntry( "mapFilename", hardware->midiMapper->url() );

	delete midiConfig;

	config->sync();
}

void KArtsModule::load()
{
	GetSettings();
}

void KArtsModule::save()
{
	if (configChanged) {
		configChanged = false;
		saveParams();
		restartServer();
		updateWidgets();
	}
	emit changed( false );
}

int KArtsModule::userSavedChanges()
{
	int reply;

	if (!configChanged)
		return KMessageBox::Yes;

	QString question = i18n("The settings have changed since the last time "
                            "you restarted the sound server.\n"
                            "Do you want to save them?");
	QString caption = i18n("Save Sound Server Settings?");
	reply = KMessageBox::questionYesNo(this, question, caption,KStdGuiItem::save(),KStdGuiItem::discard());
	if ( reply == KMessageBox::Yes)
	{
        configChanged = false;
        saveParams();
	}

    return reply;
}

void KArtsModule::slotTestSound()
{
	if (configChanged && (userSavedChanges() == KMessageBox::Yes) || !artsdIsRunning() )
		restartServer();

	KProcess test;
	test << "artsplay";
	test << locate("sound", "KDE_Startup_1.ogg");
	test.start(KProcess::DontCare);
}

void KArtsModule::slotTestMIDI()
{
	if (configChanged && (userSavedChanges() == KMessageBox::Yes) || !artsdIsRunning() )
		restartServer();

	//KProcess test;
	//test << "artsplay";
	//test << locate("sound", "KDE_Startup_1.ogg");
	//test.start(KProcess::DontCare);
}


void KArtsModule::defaults()
{
	startServer->setChecked(true);
	startRealtime->setChecked(startRealtime->isEnabled() &&
	                          realtimeIsPossible());
	networkTransparent->setChecked(false);
	fullDuplex->setChecked(false);
	autoSuspend->setChecked(true);
	suspendTime->setValue(60);
	customDevice->setChecked(false);
	deviceName->setText(QString::null);
	customRate->setChecked(false);
	samplingRate->setValue(44100);
	general->latencySlider->setValue(250);
	hardware->customOptions->setChecked(false);
	hardware->addOptions->setText(QString::null);
	hardware->audioIO->setCurrentItem(0);
	hardware->soundQuality->setCurrentItem(0);
	hardware->midiUseMapper->setChecked(false);
	hardware->midiMapper->lineEdit()->clear();

	slotChanged();
}

void KArtsModule::calculateLatency()
{
	int latencyInBytes, latencyInMs;

	if(general->latencySlider->value() < 490)
	{
		int rate = customRate->isChecked() ? samplingRate->text().toLong() : 44100;

		if (rate < 4000 || rate > 200000) {
			rate = 44100;
		}

		int sampleSize = (hardware->soundQuality->currentItem() == 2) ? 2 : 4;

		latencyInBytes = general->latencySlider->value()*rate*sampleSize/1000;

		fragmentSize = 2;
		do {
			fragmentSize *= 2;
			fragmentCount = latencyInBytes / fragmentSize;
		} while (fragmentCount > 8 && fragmentSize != 4096);

		latencyInMs = (fragmentSize*fragmentCount*1000) / rate / sampleSize;
		general->latencyLabel->setText(
						  i18n("%1 milliseconds (%2 fragments with %3 bytes)")
						  .arg(latencyInMs).arg(fragmentCount).arg(fragmentSize));
	}
	else
	{
		fragmentCount = 128;
		fragmentSize = 8192;
		general->latencyLabel->setText(i18n("as large as possible"));
	}
}

void KArtsModule::updateWidgets()
{
	bool startServerIsChecked = startServer->isChecked();
	if (startRealtime->isChecked() && !realtimeIsPossible()) {
		startRealtime->setChecked(false);
		KMessageBox::error(this, i18n("Impossible to start aRts with realtime "
		                              "priority because artswrapper is "
		                              "missing or disabled"));
	}
	deviceName->setEnabled(customDevice->isChecked());
	QString audioIO;
	int item = hardware->audioIO->currentItem() - 1;	// first item: "default"
	if (item >= 0)
	{
		audioIO = audioIOList.at(item)->name;
		bool jack = (audioIO == QLatin1String("jack"));
		if(jack)
		{
			customRate->setChecked(false);
			hardware->soundQuality->setCurrentItem(0);
			autoSuspend->setChecked(false);
		}
		customRate->setEnabled(!jack);
		hardware->soundQuality->setEnabled(!jack);
		autoSuspend->setEnabled(!jack);
	}
	samplingRate->setEnabled(customRate->isChecked());
	hardware->addOptions->setEnabled(hardware->customOptions->isChecked());
	suspendTime->setEnabled(autoSuspend->isChecked());
	calculateLatency();

	general->testSound->setEnabled(startServerIsChecked);
	general->testMIDI->setEnabled(startServerIsChecked);

//	general->volumeSystray->setEnabled(startServerIsChecked);
	general->networkedSoundGroupBox->setEnabled(startServerIsChecked);
	general->realtimeGroupBox->setEnabled(startServerIsChecked);
	general->autoSuspendGroupBox->setEnabled(startServerIsChecked);
	hardware->setEnabled(startServerIsChecked);
	hardware->midiMapper->setEnabled( hardware->midiUseMapper->isChecked() );
}

void KArtsModule::slotChanged()
{
	updateWidgets();
	configChanged = true;
	emit changed(true);
}

/* check if starting realtime would be possible */
bool KArtsModule::realtimeIsPossible()
{
	static bool checked = false;
	if (!checked)
	{
	KProcess* checkProcess = new KProcess();
	*checkProcess << "artswrapper";
	*checkProcess << "check";

	connect(checkProcess, SIGNAL(processExited(KProcess*)),
	        this, SLOT(slotArtsdExited(KProcess*)));
	if (!checkProcess->start(KProcess::Block))
	{
		delete checkProcess;
		realtimePossible =  false;
	}
	else if (latestProcessStatus == 0)
	{
		realtimePossible =  true;
	}
	else
	{
		realtimePossible =  false;
	}

	checked = true;

	}
	return realtimePossible;
}

void KArtsModule::restartServer()
{
	config->setGroup("Arts");
        bool starting = config->readBoolEntry("StartServer", true);
	bool restarting = artsdIsRunning();

	// Shut down knotify
	DCOPRef("knotify", "qt/knotify").send("quit");

	// Shut down artsd
	KProcess terminateArts;
	terminateArts << "artsshell";
	terminateArts << "terminate";
	terminateArts.start(KProcess::Block);

	if (starting)
	{
		// Wait for artsd to shutdown completely and then (re)start artsd again
		KStartArtsProgressDialog dlg(this, "start_arts_progress",
	                       restarting ? i18n("Restarting Sound System") : i18n("Starting Sound System"),
	                       restarting ? i18n("Restarting sound system.") : i18n("Starting sound system."));
	        dlg.exec();
	}

	// Restart knotify
	kapp->startServiceByDesktopName("knotify");
}

bool KArtsModule::artsdIsRunning()
{
	KProcess check;
	check << "artsshell";
	check << "status";
	check.start(KProcess::Block);

	return (check.exitStatus() == 0);
}


void init_arts()
{
	startArts();
}

QString KArtsModule::createArgs(bool netTrans,
                                bool duplex, int fragmentCount,
                                int fragmentSize,
                                const QString &deviceName,
                                int rate, int bits, const QString &audioIO,
                                const QString &addOptions, bool autoSuspend,
                                int suspendTime
                                )
{
	QString args;

	if(fragmentCount)
		args += QString::fromLatin1(" -F %1").arg(fragmentCount);

	if(fragmentSize)
		args += QString::fromLatin1(" -S %1").arg(fragmentSize);

	if (!audioIO.isEmpty())
		args += QString::fromLatin1(" -a %1").arg(audioIO);

	if (duplex)
		args += QLatin1String(" -d");

	if (netTrans)
		args += QLatin1String(" -n");

	if (!deviceName.isEmpty())
		args += QLatin1String(" -D ") + deviceName;

	if (rate)
		args += QString::fromLatin1(" -r %1").arg(rate);

	if (bits)
		args += QString::fromLatin1(" -b %1").arg(bits);

	if (autoSuspend && suspendTime)
		args += QString::fromLatin1(" -s %1").arg(suspendTime);

	if (!addOptions.isEmpty())
		args += QChar(' ') + addOptions;

	args += QLatin1String(" -m artsmessage");
	args += QLatin1String(" -c drkonqi");
	args += QLatin1String(" -l 3");
	args += QLatin1String(" -f");

	return args;
}

KStartArtsProgressDialog::KStartArtsProgressDialog(KArtsModule *parent, const char *name,
                          const QString &caption, const QString &text)
 : KProgressDialog(parent, name, caption, text, true), m_module(parent), m_shutdown(false)
{
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotProgress()));
  progressBar()->setTotalSteps(20);
  m_timeStep = 700;
  m_timer.start(m_timeStep);
  setAutoClose(false);
}

void
KStartArtsProgressDialog::slotProgress()
{
  int p = progressBar()->progress();
  if (p == 18)
  {
     progressBar()->reset();
     progressBar()->setProgress(1);
     m_timeStep = m_timeStep * 2;
     m_timer.start(m_timeStep);
  }
  else
  {
     progressBar()->setProgress(p+1);
  }

  if (!m_shutdown)
  {
     // Wait for arts to shutdown
     if (!m_module->artsdIsRunning())
     {
     	// Shutdown complete, restart
     	if (!startArts())
     		slotFinished(); // Strange, it didn't start
     	else
	     	m_shutdown = true;
     }
  }
  
  // Shut down completed? Wait for artsd to come up again
  if (m_shutdown && m_module->artsdIsRunning())
     slotFinished(); // Restart complete
}

void
KStartArtsProgressDialog::slotFinished()
{
  progressBar()->setProgress(20);
  m_timer.stop();
  QTimer::singleShot(1000, this, SLOT(close()));
}


#ifdef I18N_ONLY
	//lukas: these are hacks to allow translation of the following
	I18N_NOOP("No Audio Input/Output");
	I18N_NOOP("Advanced Linux Sound Architecture");
	I18N_NOOP("Open Sound System");
	I18N_NOOP("Threaded Open Sound System");
	I18N_NOOP("Network Audio System");
	I18N_NOOP("Personal Audio Device");
	I18N_NOOP("SGI dmedia Audio I/O");
	I18N_NOOP("Sun Audio Input/Output");
	I18N_NOOP("Portable Audio Library");
	I18N_NOOP("Enlightened Sound Daemon");
	I18N_NOOP("MAS Audio Input/Output");
	I18N_NOOP("Jack Audio Connection Kit");
#endif

#include "arts.moc"
