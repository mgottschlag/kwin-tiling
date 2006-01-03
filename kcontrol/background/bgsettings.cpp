/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#include <netwm.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// Clean up after X.h/Xlib.h
#undef Bool
#undef Unsorted

#include <qdir.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QByteArray>

#include <dcopclient.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <krandomsequence.h>
#include <konq_defaults.h>

#include "bgdefaults.h"
#include "bgsettings.h"
#include <QX11Info>


/*
 * QString -> int hash. From Qt's QGDict::hashKeyString().
 */

static int BGHash(QString key)
{
    int g, h = 0;
    const QChar *p = key.unicode();
    for (int i=0; i < key.length(); i++) {
        h = (h << 4) + p[i].cell();
        if ((g = (h & 0xf0000000)))
            h ^= (g >> 24);
        h &= ~g;
    }
    return h;
}


/**** KBackgroundPattern ****/


KBackgroundPattern::KBackgroundPattern(QString name)
{
    dirty = false;
    hashdirty = true;

    m_pDirs = KGlobal::dirs();
    m_pDirs->addResourceType("dtop_pattern", m_pDirs->kde_default("data") +
                             "kdesktop/patterns");
    m_pConfig = 0L;

    m_Name = name;
    if (m_Name.isEmpty())
        return;

    init();
    readSettings();
}


KBackgroundPattern::~KBackgroundPattern()
{
    delete m_pConfig;
}

void KBackgroundPattern::copyConfig(const KBackgroundPattern *settings)
{
    dirty = hashdirty = true;
    m_Name = settings->m_Name;
    m_Comment = settings->m_Comment;
    m_Pattern = settings->m_Pattern;
    m_File = settings->m_File;
}

void KBackgroundPattern::load(QString name)
{
    m_Name = name;
    init();
    readSettings();
}


void KBackgroundPattern::init(bool force_rw)
{
    delete m_pConfig;

    m_File = m_pDirs->findResource("dtop_pattern", m_Name + ".desktop");
    if (force_rw || m_File.isEmpty()) {
        m_File = m_pDirs->saveLocation("dtop_pattern") + m_Name + ".desktop";
        m_pConfig = new KSimpleConfig(m_File);
    } else
        m_pConfig = new KSimpleConfig(m_File);

    m_pConfig->setGroup("KDE Desktop Pattern");

    QFileInfo fi(m_File);
    m_bReadOnly = !fi.isWritable();
}


void KBackgroundPattern::setComment(const QString &comment)
{
    if (m_Comment == comment)
        return;
    dirty = true;
    m_Comment = comment;
}


void KBackgroundPattern::setPattern(QString pattern)
{
    if (m_Pattern == pattern)
        return;
    dirty = hashdirty = true;
    m_Pattern = pattern;
}


void KBackgroundPattern::readSettings()
{
    dirty = false;
    hashdirty = true;

    m_Pattern = m_pConfig->readPathEntry("File");
    m_Comment = m_pConfig->readEntry("Comment");
    if (m_Comment.isEmpty())
       m_Comment = m_File.mid(m_File.lastIndexOf('/')+1);
}


void KBackgroundPattern::writeSettings()
{
    if (!dirty)
        return;
    if (m_bReadOnly)
        init(true);

    if ( !m_pConfig )
        return; // better safe than sorry

    m_pConfig->writePathEntry("File", m_Pattern);
    m_pConfig->writeEntry("Comment", m_Comment);
    m_pConfig->sync();
    dirty = false;
}


bool KBackgroundPattern::isAvailable()
{
    if (m_Pattern.isEmpty())
       return false;
    QString file = m_Pattern;
    if (file.at(0) != '/')
        file = m_pDirs->findResource("dtop_pattern", file);
    QFileInfo fi(file);
    return (fi.exists());
}


bool KBackgroundPattern::remove()
{
    if (m_bReadOnly)
        return false;
    return !unlink(QFile::encodeName(m_File));
}


QString KBackgroundPattern::fingerprint()
{
    return m_File;
}


int KBackgroundPattern::hash()
{
    if (hashdirty) {
	m_Hash = BGHash(fingerprint());
	hashdirty = false;
    }
    return m_Hash;
}


/* static */
QStringList KBackgroundPattern::list()
{
    KStandardDirs *dirs = KGlobal::dirs();
    dirs->addResourceType("dtop_pattern", dirs->kde_default("data") +
                          "kdesktop/patterns");
    QStringList lst = dirs->findAllResources("dtop_pattern", "*.desktop",
                                             false, true);
    QStringList::Iterator it;
    for (it=lst.begin(); it!=lst.end(); ++it) {
        // Strip path and suffix
        int pos = (*it).lastIndexOf('/');
        if (pos != -1)
            (*it) = (*it).mid(pos+1);
        pos = (*it).lastIndexOf('.');
        if (pos != -1)
            (*it) = (*it).left(pos);
    }
    return lst;
}


/**** KBackgroundProgram ****/


KBackgroundProgram::KBackgroundProgram(QString name)
{
    dirty = false;
    hashdirty = true;

    m_pDirs = KGlobal::dirs();
    m_pDirs->addResourceType("dtop_program", m_pDirs->kde_default("data") +
                             "kdesktop/programs");
    m_pConfig = 0L;

    // prevent updates when just constructed.
    m_LastChange = (int) time(0L);

    m_Name = name;
    if (m_Name.isEmpty())
        return;

    init();
    readSettings();
}


KBackgroundProgram::~KBackgroundProgram()
{
    delete m_pConfig;
}

void KBackgroundProgram::copyConfig(const KBackgroundProgram *settings)
{
    dirty = hashdirty = true;
    m_Refresh = settings->m_Refresh;
    m_LastChange = settings->m_LastChange;
    m_Name = settings->m_Name;
    m_Command = settings->m_Command;
    m_PreviewCommand = settings->m_PreviewCommand;
    m_Comment = settings->m_Comment;
    m_Executable = settings->m_Executable;
    m_File = settings->m_File;
}


void KBackgroundProgram::init(bool force_rw)
{
    delete m_pConfig;

    m_File = m_pDirs->findResource("dtop_program", m_Name + ".desktop");
    if (force_rw || m_File.isEmpty()) {
        m_File = m_pDirs->saveLocation("dtop_program") + m_Name + ".desktop";
        m_pConfig = new KSimpleConfig(m_File);
        m_bReadOnly = false;
    } else {
        m_pConfig = new KSimpleConfig(m_File);
        m_bReadOnly = (m_File != locateLocal("dtop_program", m_Name + ".desktop"));
    }
    m_pConfig->setGroup("KDE Desktop Program");
}


void KBackgroundProgram::load(const QString &name)
{
    m_Name = name;
    init();
    readSettings();
}


void KBackgroundProgram::setComment(const QString &comment)
{
    if (m_Comment == comment)
        return;
    dirty = true;
    m_Comment = comment;
}


void KBackgroundProgram::setExecutable(const QString &executable)
{
    if (m_Executable == executable)
        return;
    dirty = true;
    m_Executable = executable;
}


void KBackgroundProgram::setCommand(const QString &command)
{
    if (m_Command == command)
        return;
    dirty = hashdirty = true;
    m_Command = command;
}


void KBackgroundProgram::setPreviewCommand(const QString &command)
{
    if (m_PreviewCommand == command)
        return;
    dirty = true;
    m_PreviewCommand = command;
}


void KBackgroundProgram::setRefresh(int refresh)
{
    if (m_Refresh == refresh)
        return;
    dirty = hashdirty = true;
    m_Refresh = refresh;
}


void KBackgroundProgram::readSettings()
{
    dirty = false;
    hashdirty = true;

    m_Comment = m_pConfig->readEntry("Comment");
    m_Executable = m_pConfig->readPathEntry("Executable");
    m_Command = m_pConfig->readPathEntry("Command");
    m_PreviewCommand = m_pConfig->readPathEntry("PreviewCommand", m_Command);
    m_Refresh = m_pConfig->readNumEntry("Refresh", 300);
}


void KBackgroundProgram::writeSettings()
{
    if (!dirty)
        return;
    if (m_bReadOnly)
        init(true);

    if ( !m_pConfig )
        return; // better safe than sorry

    m_pConfig->writeEntry("Comment", m_Comment);
    m_pConfig->writePathEntry("Executable", m_Executable);
    m_pConfig->writePathEntry("Command", m_Command);
    m_pConfig->writeEntry("PreviewCommand", m_PreviewCommand);
    m_pConfig->writeEntry("Refresh", m_Refresh);
    m_pConfig->sync();
    dirty = false;
}


bool KBackgroundProgram::isAvailable()
{
    return !m_pDirs->findExe(m_Executable).isEmpty();
}


bool KBackgroundProgram::remove()
{
    if (m_bReadOnly)
        return false;
    return !unlink(QFile::encodeName(m_File));
}


bool KBackgroundProgram::needUpdate()
{
    return (m_LastChange + 60*m_Refresh <= time(0L));
}


void KBackgroundProgram::update()
{
    m_LastChange = (int) time(0L);
}


QString KBackgroundProgram::fingerprint()
{
    return QString("co:%1;re:%2").arg(m_Command).arg(m_Refresh);
}


int KBackgroundProgram::hash()
{
    if (hashdirty) {
	m_Hash = BGHash(fingerprint());
	hashdirty = false;
    }
    return m_Hash;
}


/* static */
QStringList KBackgroundProgram::list()
{
    KStandardDirs *dirs = KGlobal::dirs();
    dirs->addResourceType("dtop_program", dirs->kde_default("data") +
                          "kdesktop/programs");
    QStringList lst = dirs->findAllResources("dtop_program", "*.desktop",
                                             false, true);
    QStringList::Iterator it;
    for (it=lst.begin(); it!=lst.end(); ++it) {
        // Strip path and suffix
        int pos = (*it).lastIndexOf('/');
        if (pos != -1)
            (*it) = (*it).mid(pos+1);
        pos = (*it).lastIndexOf('.');
        if (pos != -1)
            (*it) = (*it).left(pos);
    }
    return lst;
}


/**** KBackgroundSettings ****/


KBackgroundSettings::KBackgroundSettings(int desk, KConfig *config)
    : KBackgroundPattern(),
      KBackgroundProgram()
{
    dirty = false; hashdirty = true;
    m_Desk = desk;
    m_bEnabled = true;

    // Default values.
    defColorA = _defColorA;
    defColorB = _defColorB;
    if (QPixmap::defaultDepth() > 8)
        defBackgroundMode = _defBackgroundMode;
    else
        defBackgroundMode = Flat;
    defWallpaperMode = _defWallpaperMode;
    defMultiMode = _defMultiMode;
    defBlendMode = _defBlendMode;
    defBlendBalance = _defBlendBalance;
    defReverseBlending = _defReverseBlending;

    m_MinOptimizationDepth = _defMinOptimizationDepth;
    m_bShm = _defShm;

    // Background modes
    #define ADD_STRING(ID) m_BMMap[#ID] = ID; m_BMRevMap[ID] = (char *) #ID;
    ADD_STRING(Flat)
    ADD_STRING(Pattern)
    ADD_STRING(Program)
    ADD_STRING(HorizontalGradient)
    ADD_STRING(VerticalGradient)
    ADD_STRING(PyramidGradient)
    ADD_STRING(PipeCrossGradient)
    ADD_STRING(EllipticGradient)
    #undef ADD_STRING

    // Blend modes
    #define ADD_STRING(ID) m_BlMMap[#ID] = ID; m_BlMRevMap[ID] = (char *) #ID;
    ADD_STRING(NoBlending)
    ADD_STRING(FlatBlending)
    ADD_STRING(HorizontalBlending)
    ADD_STRING(VerticalBlending)
    ADD_STRING(PyramidBlending)
    ADD_STRING(PipeCrossBlending)
    ADD_STRING(EllipticBlending)
    ADD_STRING(IntensityBlending)
    ADD_STRING(SaturateBlending)
    ADD_STRING(ContrastBlending)
    ADD_STRING(HueShiftBlending)
    #undef ADD_STRING

    // Wallpaper modes
    #define ADD_STRING(ID) m_WMMap[#ID] = ID; m_WMRevMap[ID] = (char *) #ID;
    ADD_STRING(NoWallpaper)
    ADD_STRING(Centred)
    ADD_STRING(Tiled)
    ADD_STRING(CenterTiled)
    ADD_STRING(CentredMaxpect)
    ADD_STRING(TiledMaxpect)
    ADD_STRING(Scaled)
    ADD_STRING(CentredAutoFit)
    ADD_STRING(ScaleAndCrop)
    #undef ADD_STRING

    // Multiple wallpaper modes
    #define ADD_STRING(ID) m_MMMap[#ID] = ID; m_MMRevMap[ID] = (char *) #ID;
    ADD_STRING(NoMulti)
    ADD_STRING(InOrder)
    ADD_STRING(Random)
    ADD_STRING(NoMultiRandom)
    #undef ADD_STRING

    m_pDirs = KGlobal::dirs();

    if (!config) {
        int screen_number = 0;
        if (QX11Info::display())
            screen_number = DefaultScreen(QX11Info::display());
        QString configname;
        if (screen_number == 0)
            configname = "kdesktoprc";
        else
            configname.sprintf("kdesktop-screen-%drc", screen_number);

        m_pConfig = new KConfig(configname, false, false);
        m_bDeleteConfig = true;
    } else {
        m_pConfig = config;
        m_bDeleteConfig = false;
    }

    if (m_Desk == -1)
	return;

    readSettings();
}


KBackgroundSettings::~KBackgroundSettings()
{
    if (m_bDeleteConfig)
        delete m_pConfig;
}

void KBackgroundSettings::copyConfig(const KBackgroundSettings *settings)
{
    dirty = true;
    hashdirty = true;

    m_ColorA = settings->m_ColorA;
    m_ColorB = settings->m_ColorB;
    m_Wallpaper = settings->m_Wallpaper;
    m_WallpaperList = settings->m_WallpaperList;
    m_WallpaperFiles = settings->m_WallpaperFiles;

    m_BackgroundMode = settings->m_BackgroundMode;
    m_WallpaperMode = settings->m_WallpaperMode;
    m_BlendMode = settings->m_BlendMode;
    m_BlendBalance = settings->m_BlendBalance;
    m_ReverseBlending = settings->m_ReverseBlending;
    m_MinOptimizationDepth = settings->m_MinOptimizationDepth;
    m_bShm = settings->m_bShm;
    m_MultiMode = settings->m_MultiMode;
    m_Interval = settings->m_Interval;
    m_CurrentWallpaper = settings->m_CurrentWallpaper;

    KBackgroundPattern::copyConfig(settings);
    KBackgroundProgram::copyConfig(settings);
}


void KBackgroundSettings::load(int desk, bool reparseConfig)
{
    m_Desk = desk;
    readSettings(reparseConfig);
}


void KBackgroundSettings::setColorA(const QColor& color)
{
    if (m_ColorA == color)
        return;
    dirty = hashdirty = true;
    m_ColorA = color;
}


void KBackgroundSettings::setColorB(const QColor& color)
{
    if (m_ColorB == color)
        return;
    dirty = hashdirty = true;
    m_ColorB = color;
}


void KBackgroundSettings::setPatternName(QString name)
{
    int ohash = KBackgroundPattern::hash();
    KBackgroundPattern::load(name);
    if (ohash == KBackgroundPattern::hash())
	return;

    dirty = hashdirty = true;
    return;
}


void KBackgroundSettings::setProgram(QString name)
{
    int ohash = KBackgroundProgram::hash();
    KBackgroundProgram::load(name);
    if (ohash == KBackgroundProgram::hash())
	return;

    dirty = hashdirty = true;
    return;
}


void KBackgroundSettings::setBackgroundMode(int mode)
{
    if (m_BackgroundMode == mode)
	return;
    dirty = hashdirty = true;
    m_BackgroundMode = mode;
}

void KBackgroundSettings::setBlendMode(int mode)
{
    if (m_BlendMode == mode)
	return;
    dirty = hashdirty = true;
    m_BlendMode = mode;
}

void KBackgroundSettings::setBlendBalance(int value)
{
    if (m_BlendBalance == value)
	return;
    dirty = hashdirty = true;
    m_BlendBalance = value;
}

void KBackgroundSettings::setReverseBlending(bool value)
{
    if (m_ReverseBlending == value)
	return;
    dirty = hashdirty = true;
    m_ReverseBlending = value;
}


void KBackgroundSettings::setWallpaper(QString wallpaper)
{
    if (m_Wallpaper == wallpaper)
        return;
    dirty = hashdirty = true;
    m_Wallpaper = wallpaper;
}


void KBackgroundSettings::setWallpaperMode(int mode)
{
    if (m_WallpaperMode == mode)
        return;
    dirty = hashdirty = true;
    m_WallpaperMode = mode;
}


void KBackgroundSettings::setWallpaperList(QStringList list)
{
    KStandardDirs *d = KGlobal::dirs();
    if (m_WallpaperList == list)
	return;

    QString currentWallpaper;
    if ((m_MultiMode == InOrder) && (m_CurrentWallpaper < (int) m_WallpaperFiles.count()))
	currentWallpaper = m_WallpaperFiles[m_CurrentWallpaper];

    dirty = hashdirty = true;
    m_WallpaperList.clear();
    for(QStringList::ConstIterator it = list.begin();
        it != list.end(); ++it)
    {
       QString rpath = d->relativeLocation("wallpaper", *it);
       m_WallpaperList.append( !rpath.isEmpty() ? rpath : *it );
    }
    updateWallpaperFiles();
    // Try to keep the current wallpaper
    m_CurrentWallpaper = m_WallpaperFiles.findIndex(currentWallpaper) - 1;
    changeWallpaper(m_CurrentWallpaper < 0);
}


void KBackgroundSettings::setWallpaperChangeInterval(int interval)
{
    if (m_Interval == interval)
	return;
    dirty = hashdirty = true;
    m_Interval = interval;
}


void KBackgroundSettings::setMultiWallpaperMode(int mode)
{
    if (m_MultiMode == mode)
	return;
    dirty = hashdirty = true;
    m_MultiMode = mode;
    changeWallpaper(true);
}


void KBackgroundSettings::setMinOptimizationDepth(int mode)
{
    if (m_MinOptimizationDepth == mode)
	return;
    dirty = hashdirty = true;
    m_MinOptimizationDepth = mode;
}

bool KBackgroundSettings::optimize() const
{
    switch( m_MinOptimizationDepth )
        {
        case AlwaysOpt :
            return true;
        case Opt16bpp :
            return QPixmap::defaultDepth() >= 16;
        case Opt15bpp :
            return QPixmap::defaultDepth() >= 15;
        case NeverOpt :
        default :
            return false;
        }
}

void KBackgroundSettings::setUseShm(bool use)
{
    if (m_bShm == use)
	return;
    dirty = hashdirty = true;
    m_bShm = use;
}

void KBackgroundSettings::readSettings(bool reparse)
{
    if (reparse)
        m_pConfig->reparseConfiguration();

    m_pConfig->setGroup(QString("Desktop%1").arg(m_Desk));

    // Background mode (Flat, div. Gradients, Pattern or Program)
    m_ColorA = m_pConfig->readColorEntry("Color1", &defColorA);
    m_ColorB = m_pConfig->readColorEntry("Color2", &defColorB);

    QString s = m_pConfig->readPathEntry("Pattern");
    if (!s.isEmpty())
        KBackgroundPattern::load(s);

    s = m_pConfig->readPathEntry("Program");
    if (!s.isEmpty())
        KBackgroundProgram::load(s);

    m_BackgroundMode = defBackgroundMode;
    s = m_pConfig->readEntry("BackgroundMode", "invalid");

    if (m_BMMap.contains(s)) {
        int mode = m_BMMap[s];
        // consistency check
        if  ( ((mode != Pattern) && (mode != Program)) ||
              ((mode == Pattern) && !pattern().isEmpty()) ||
              ((mode == Program) && !command().isEmpty())
            )
            m_BackgroundMode = mode;
    }

    m_BlendMode = defBlendMode;
    s = m_pConfig->readEntry("BlendMode", "invalid");
    if (m_BlMMap.contains(s)) {
      m_BlendMode = m_BlMMap[s];
    }

    m_BlendBalance = defBlendBalance;
    int value = m_pConfig->readNumEntry( "BlendBalance", defBlendBalance);
    if (value > -201 && value < 201)
      m_BlendBalance = value;

    m_ReverseBlending = m_pConfig->readEntry( "ReverseBlending", QVariant(defReverseBlending)).toBool();

    // Multiple wallpaper config
    m_WallpaperList = m_pConfig->readPathListEntry("WallpaperList");

    m_Interval = m_pConfig->readNumEntry("ChangeInterval", 60);
    m_LastChange = m_pConfig->readNumEntry("LastChange", 0);
    m_CurrentWallpaper = m_pConfig->readNumEntry("CurrentWallpaper", 0);

    m_MultiMode = defMultiMode;
    s = m_pConfig->readEntry("MultiWallpaperMode");
    if (m_MMMap.contains(s)) {
	int mode = m_MMMap[s];
	m_MultiMode = mode;
    }

    updateWallpaperFiles();

    // Wallpaper mode (NoWallpaper, div. tilings)
    m_WallpaperMode = defWallpaperMode;
    m_Wallpaper = m_pConfig->readPathEntry("Wallpaper");
    s = m_pConfig->readEntry("WallpaperMode", "invalid");
    if (m_WMMap.contains(s)) {
        int mode = m_WMMap[s];
        // consistency check.
        if ((mode == NoWallpaper) || !m_Wallpaper.isEmpty() || (m_MultiMode == InOrder || m_MultiMode == Random))
            m_WallpaperMode = mode;
    }

    m_MinOptimizationDepth = m_pConfig->readNumEntry( "MinOptimizationDepth",
        _defMinOptimizationDepth );
    m_bShm = m_pConfig->readEntry( "UseSHM", QVariant(_defShm )).toBool();

    dirty = false; hashdirty = true;
}


void KBackgroundSettings::writeSettings()
{
    KBackgroundPattern::writeSettings();
    KBackgroundProgram::writeSettings();

    if (!dirty)
        return;

    m_pConfig->setGroup(QString("Desktop%1").arg(m_Desk));
    m_pConfig->writeEntry("Color1", m_ColorA);
    m_pConfig->writeEntry("Color2", m_ColorB);
    m_pConfig->writePathEntry("Pattern", KBackgroundPattern::name());
    m_pConfig->writeEntry("Program", KBackgroundProgram::name());
    m_pConfig->writeEntry("BackgroundMode", m_BMRevMap[m_BackgroundMode]);
    m_pConfig->writePathEntry("Wallpaper", m_Wallpaper);
    m_pConfig->writeEntry("WallpaperMode", m_WMRevMap[m_WallpaperMode]);
    m_pConfig->writeEntry("MultiWallpaperMode", m_MMRevMap[m_MultiMode]);
    m_pConfig->writeEntry("BlendMode", m_BlMRevMap[m_BlendMode]);
    m_pConfig->writeEntry("BlendBalance", m_BlendBalance);
    m_pConfig->writeEntry("ReverseBlending", m_ReverseBlending);
    m_pConfig->writeEntry("MinOptimizationDepth", m_MinOptimizationDepth);
    m_pConfig->writeEntry("UseSHM", m_bShm);

    m_pConfig->writePathEntry("WallpaperList", m_WallpaperList);
    m_pConfig->writeEntry("ChangeInterval", m_Interval);
    m_pConfig->writeEntry("LastChange", m_LastChange);
    m_pConfig->writeEntry("CurrentWallpaper", m_CurrentWallpaper);

    m_pConfig->sync();

    dirty = false;
}

/*
 * (re)Build m_WallpaperFiles from m_WallpaperList
 */
void KBackgroundSettings::updateWallpaperFiles()
{
    QStringList::Iterator it;
    m_WallpaperFiles.clear();
    for (it=m_WallpaperList.begin(); it!=m_WallpaperList.end(); ++it) {
        QString file = locate("wallpaper", *it);
        if (file.isEmpty())
            continue;
	QFileInfo fi(file);
	if (!fi.exists())
	    continue;
	if (fi.isFile() && fi.isReadable())
	    m_WallpaperFiles.append(file);
	if (fi.isDir()) {
	    QDir dir(file);
	    QStringList lst = dir.entryList(QDir::Files | QDir::Readable);
	    QStringList::Iterator it;
	    for (it=lst.begin(); it!=lst.end(); ++it)
	    {
		file = dir.absoluteFilePath(*it);
		QFileInfo fi(file);
		if (fi.isFile() && fi.isReadable())
		    m_WallpaperFiles.append(file);
	    }
	}
    }

   if (m_MultiMode == Random)
       randomizeWallpaperFiles();
}

// Randomize the m_WallpaperFiles in a non-repeating method.
void KBackgroundSettings::randomizeWallpaperFiles()
{
   if (m_WallpaperFiles.count() < 4)
      return;

   KRandomSequence rseq;
   QStringList tmpList = m_WallpaperFiles;
   QStringList randomList;
   randomList.append(tmpList.front());
   tmpList.pop_front();
   while(tmpList.count())
   {
      long t = rseq.getLong(randomList.count()+1);
      randomList.insert(t, tmpList.front());
      tmpList.pop_front();
   }
   m_WallpaperFiles = randomList;
}

QStringList KBackgroundSettings::wallpaperList() const
{
    if ( m_WallpaperMode == NoWallpaper )
        return QStringList();
    if ( m_MultiMode == NoMulti || m_MultiMode == NoMultiRandom )
        return QStringList(m_Wallpaper);
    return m_WallpaperList;
}

QStringList KBackgroundSettings::wallpaperFiles() const
{
    if ( m_WallpaperMode == NoWallpaper )
        return QStringList();
    if ( m_MultiMode == NoMulti || m_MultiMode == NoMultiRandom )
        return QStringList(m_Wallpaper);
    return m_WallpaperFiles;
}

/*
 * Select a new wallpaper from the list.
 */
void KBackgroundSettings::changeWallpaper(bool init)
{
    if (m_WallpaperFiles.count() == 0) {
        if( init )
	    m_CurrentWallpaper = 0;
        return;
    }

    switch (m_MultiMode) {
    case InOrder:
	m_CurrentWallpaper++;
	if (init || (m_CurrentWallpaper >= (int) m_WallpaperFiles.count()))
	    m_CurrentWallpaper = 0;
	break;

    case Random:
      // Random: m_WallpaperFiles is randomized in a non-repeating
      //  method.  Hence we just increment the index.
      m_CurrentWallpaper++;
      if (init || (m_CurrentWallpaper >= (int) m_WallpaperFiles.count())) {
         m_CurrentWallpaper = 0;
         randomizeWallpaperFiles(); // Get a new random-ordered list.
      }
	break;
    default:
	break;
    }

    m_LastChange = (int) time(0L);
    m_pConfig->setGroup(QString("Desktop%1").arg(m_Desk));
    m_pConfig->writeEntry("CurrentWallpaper", m_CurrentWallpaper);
    m_pConfig->writeEntry("LastChange", m_LastChange);
    m_pConfig->sync();

    hashdirty = true;
}


QString KBackgroundSettings::currentWallpaper()
{
    if ( m_WallpaperMode == NoWallpaper )
	return QString();
    if (m_MultiMode == NoMulti || m_MultiMode == NoMultiRandom)
	return m_Wallpaper;
    if (m_CurrentWallpaper < (int) m_WallpaperFiles.count())
	return m_WallpaperFiles[m_CurrentWallpaper];
    return QString();
}

bool KBackgroundSettings::discardCurrentWallpaper()
{
    if (m_MultiMode == NoMulti || m_MultiMode == NoMultiRandom)
    {
       return false;
    }
    m_WallpaperFiles.remove(m_WallpaperFiles.at(m_CurrentWallpaper));
    if (m_CurrentWallpaper >= (int) m_WallpaperFiles.count())
	m_CurrentWallpaper = 0;

    return true;
}


bool KBackgroundSettings::needWallpaperChange()
{
    if (m_MultiMode == NoMulti || m_MultiMode == NoMultiRandom)
	return false;

    return ((m_LastChange + 60*m_Interval) <= time(0L));
}


/*
 * Create a fingerprint string for this config. Be somewhat (overly) carefull
 * that only a different final result will give a different fingerprint.
 */

QString KBackgroundSettings::fingerprint()
{
    QString s = QString("bm:%1;en:%2").arg(m_BackgroundMode).arg(m_bEnabled);
    switch (m_BackgroundMode) {
    case Flat:
        s += QString("ca:%1;").arg(m_ColorA.rgb());
        break;
    case Program:
        s += QString("pr:%1;").arg(KBackgroundProgram::hash());
        break;
    case Pattern:
        s += QString("ca:%1;cb:%2;pt:%3;").arg(m_ColorA.rgb())
	     .arg(m_ColorB.rgb()).arg(KBackgroundPattern::hash());
        break;
    default:
        s += QString("ca:%1;cb:%2;").arg(m_ColorA.rgb()).arg(m_ColorB.rgb());
        break;
    }

    s += QString("wm:%1;").arg(m_WallpaperMode);
    if (m_WallpaperMode != NoWallpaper)
    {
        Q_UINT32 rh = KGlobal::dirs()->calcResourceHash("wallpaper", currentWallpaper(), false);
        s += QString("wp:%2:%1;").arg(rh).arg(currentWallpaper());
        
    }
    s += QString("blm:%1;").arg(m_BlendMode);
    if (m_BlendMode != NoBlending) {
      s += QString("blb:%1;").arg(m_BlendBalance);
      s += QString("rbl:%1;").arg(int(m_ReverseBlending));
    }
    s += QString::number( m_bShm );
    s += QString::number( m_MinOptimizationDepth );

    return s;
}


int KBackgroundSettings::hash()
{
    if (hashdirty) {
        m_Hash = BGHash(fingerprint());
	hashdirty = false;
    }
    return m_Hash;
}

void KBackgroundSettings::setEnabled(const bool enable)
{
  if (m_bEnabled == enable)
    return;

  m_bEnabled= enable;
  hashdirty = true;
}

/**** KGlobalBackgroundSettings ****/

KGlobalBackgroundSettings::KGlobalBackgroundSettings(KConfig *_config)
{
    m_pConfig = _config;

    readSettings();
}


QString KGlobalBackgroundSettings::deskName(int desk)
{
    return m_Names[desk];
}


/*
void KGlobalBackgroundSettings::setDeskName(int desk, QString name)
{
    if (name == m_Names[desk])
	return;
    dirty = true;
    m_Names[desk] = name;
}
*/


void KGlobalBackgroundSettings::setCacheSize(int size)
{
    if (size == m_CacheSize)
	return;
    dirty = true;
    m_CacheSize = size;
}


void KGlobalBackgroundSettings::setLimitCache(bool limit)
{
    if (limit == m_bLimitCache)
	return;
    dirty = true;
    m_bLimitCache = limit;
}


void KGlobalBackgroundSettings::setCommonBackground(bool common)
{
    if (common == m_bCommon)
	return;
    dirty = true;
    m_bCommon = common;
}


void KGlobalBackgroundSettings::setDockPanel(bool dock)
{
    if (dock == m_bDock)
        return;
    dirty = true;
    m_bDock = dock;
}


void KGlobalBackgroundSettings::setExportBackground(bool _export)
{
    if (_export == m_bExport)
        return;
    dirty = true;
    m_bExport = _export;
}

void KGlobalBackgroundSettings::setTextColor(QColor _color)
{
    if (_color == m_TextColor)
        return;
    dirty = true;
    m_TextColor = _color;
}

void KGlobalBackgroundSettings::setTextBackgroundColor(QColor _color)
{
    if (_color == m_TextBackgroundColor)
        return;
    dirty = true;
    m_TextBackgroundColor = _color;
}

void KGlobalBackgroundSettings::setShadowEnabled(bool enabled)
{
    if (enabled == m_shadowEnabled)
        return;
    dirty = true;
    m_shadowEnabled = enabled;
}

void KGlobalBackgroundSettings::setTextLines(int lines)
{
    if (lines == m_textLines)
        return;
    dirty = true;
    m_textLines = lines;
}

void KGlobalBackgroundSettings::setTextWidth(int width)
{
    if (width == m_textWidth)
        return;
    dirty = true;
    m_textWidth = width;
}

void KGlobalBackgroundSettings::readSettings()
{
    m_pConfig->setGroup("Background Common");
    m_bCommon = m_pConfig->readEntry("CommonDesktop", QVariant(_defCommon)).toBool();
    m_bDock = m_pConfig->readEntry("Dock", QVariant(_defDock)).toBool();
    m_bExport = m_pConfig->readEntry("Export", QVariant(_defExport)).toBool();
    m_bLimitCache = m_pConfig->readEntry("LimitCache", QVariant(_defLimitCache)).toBool();
    m_CacheSize = m_pConfig->readNumEntry("CacheSize", _defCacheSize);

    m_TextColor = KGlobalSettings::textColor();
    m_pConfig->setGroup("FMSettings");
    m_TextColor = m_pConfig->readColorEntry("NormalTextColor", &m_TextColor);
    m_TextBackgroundColor = m_pConfig->readColorEntry("ItemTextBackground");
    m_shadowEnabled = m_pConfig->readEntry("ShadowEnabled", QVariant(true)).toBool();
    m_textLines = m_pConfig->readNumEntry("TextHeight", DEFAULT_TEXTHEIGHT);
    m_textWidth = m_pConfig->readNumEntry("TextWidth", DEFAULT_TEXTWIDTH);

    m_Names.clear();
    NETRootInfo info( QX11Info::display(), NET::DesktopNames | NET::NumberOfDesktops );
    for ( int i = 0 ; i < info.numberOfDesktops() ; ++i )
      m_Names.append( QString::fromUtf8(info.desktopName(i+1)) );

    dirty = false;
}

void KGlobalBackgroundSettings::writeSettings()
{
    if (!dirty)
        return;

    m_pConfig->setGroup("Background Common");
    m_pConfig->writeEntry("CommonDesktop", m_bCommon);
    m_pConfig->writeEntry("Dock", m_bDock);
    m_pConfig->writeEntry("Export", m_bExport);
    m_pConfig->writeEntry("LimitCache", m_bLimitCache);
    m_pConfig->writeEntry("CacheSize", m_CacheSize);

    m_pConfig->setGroup("FMSettings");
    m_pConfig->writeEntry("NormalTextColor", m_TextColor);
    m_pConfig->writeEntry("ItemTextBackground", m_TextBackgroundColor);
    m_pConfig->writeEntry("ShadowEnabled", m_shadowEnabled);
    m_pConfig->writeEntry("TextHeight", m_textLines);
    m_pConfig->writeEntry("TextWidth", m_textWidth);
    m_pConfig->sync();
    dirty = false;

    // tell kdesktop to get it's butt in gear and pick up the new settings
    QByteArray data;
    kapp->dcopClient()->send("kdesktop", "KDesktopIface", "configure()", data);
}

