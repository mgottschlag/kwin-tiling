//-----------------------------------------------------------------------------
//
// KDE xscreensaver configuration dialog
//
// Copyright (c)  Martin R. Jones 1999
//

#include <qlayout.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <klocale.h>
#include "kxsconfig.h"
#include "kxscontrol.h"

//===========================================================================
KXSConfigDialog::KXSConfigDialog(const QString &filename)
  : KDialogBase(Plain, filename, Ok| Cancel, Ok, 0, 0, false),
    mFilename(filename)
{
  int slash = filename.findRev('/');
  if (slash >= 0)
  {
    mConfigFile = filename.mid(slash+1);
  }
  else
  {
    mConfigFile = filename;
  }

  mConfigFile += "rc";

  KConfig config(mConfigFile);

  QHBoxLayout *layout = new QHBoxLayout(plainPage(), spacingHint());
  QVBoxLayout *controlLayout = new QVBoxLayout(layout, spacingHint());
  controlLayout->addStrut(120);

  int idx = 0;

  while (true)
  {
    QString group = QString("Arg%1").arg(idx);
    if (config.hasGroup(group))
    {
      config.setGroup(group);
      QString type = config.readEntry("Type");
      if (type == "Range")
      {
        KXSRangeControl *rc = new KXSRangeControl(plainPage(), group, config);
        connect(rc, SIGNAL(changed()), SLOT(slotChanged()));
        controlLayout->add(rc);
        mConfigItemList.append(rc);
      }
      else if (type == "Check")
      {
        KXSCheckBoxControl *cc = new KXSCheckBoxControl(plainPage(), group,
                                                        config);
        connect(cc, SIGNAL(changed()), SLOT(slotChanged()));
        controlLayout->add(cc);
        mConfigItemList.append(cc);
      }
      else if (type == "DropList")
      {
        KXSDropListControl *dl = new KXSDropListControl(plainPage(), group,
                                                        config);
        connect(dl, SIGNAL(changed()), SLOT(slotChanged()));
        controlLayout->add(dl);
        mConfigItemList.append(dl);
      }
    }
    else
    {
      break;
    }
    idx++;
  }

  controlLayout->addStretch(1);

  mPreviewProc = new KProcess;
  connect(mPreviewProc, SIGNAL(processExited(KProcess *)),
                        SLOT(slotPreviewExited(KProcess *)));

  mPreviewTimer = new QTimer(this);
  connect(mPreviewTimer, SIGNAL(timeout()), SLOT(slotNewPreview()));

  mPreview = new QWidget(plainPage());
  mPreview->setFixedSize(250, 200);

  layout->add(mPreview);

  slotPreviewExited(0);
}

//---------------------------------------------------------------------------
KXSConfigDialog::~KXSConfigDialog()
{
  if (mPreviewProc->isRunning())
  {
    int pid = mPreviewProc->getPid();
    mPreviewProc->kill();
    waitpid(pid, (int *)0, 0);
    delete mPreviewProc;
  }
}

//---------------------------------------------------------------------------
QString KXSConfigDialog::command()
{
  QString cmd;
  KXSConfigItem *item;

  for (item = mConfigItemList.first(); item != 0; item = mConfigItemList.next())
  {
    cmd += " " + item->command();
  }

  return cmd;
}

//---------------------------------------------------------------------------
void KXSConfigDialog::slotPreviewExited(KProcess *)
{
  mPreviewProc->clearArguments();

  QString saver = mFilename + " -window-id %w";
  saver += command();
  debug("Command: %s", saver.ascii());
  QTextStream ts(&saver, IO_ReadOnly);

  QString word;
  ts >> word;
  QString path = KStandardDirs::findExe(word);

  if (!path.isEmpty())
  {
    (*mPreviewProc) << path;

    while (!ts.atEnd())
    {
      ts >> word;
      word = word.stripWhiteSpace();
      if (word == "%w")
      {
        word = word.setNum(mPreview->winId());
      }
      if (!word.isEmpty())
      {
        (*mPreviewProc) << word;
      }
    }

    mPreviewProc->start();
  }
}

//---------------------------------------------------------------------------
void KXSConfigDialog::slotNewPreview()
{
  if (mPreviewProc->isRunning())
  {
    mPreviewProc->kill(); // restarted in slotPreviewExited()
  }
}

//---------------------------------------------------------------------------
void KXSConfigDialog::slotChanged()
{
  if (mPreviewTimer->isActive())
  {
    mPreviewTimer->changeInterval(1000);
  }
  else
  {
    mPreviewTimer->start(1000, true);
  }
}

//---------------------------------------------------------------------------
void KXSConfigDialog::slotOk()
{
  KXSConfigItem *item;
  KConfig config(mConfigFile);

  for (item = mConfigItemList.first(); item != 0; item = mConfigItemList.next())
  {
    item->save(config);
  }

  debug(command());
  kapp->quit();
}

//---------------------------------------------------------------------------
void KXSConfigDialog::slotCancel()
{
  kapp->quit();
}

//===========================================================================

void usage(const char *name)
{
  printf("kxsconfig - xscreensaver configuration\n");
  printf("Usage: %s xscreensaver-filename\n", name);
}


int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "KXSConfig");

  if (argc != 2 || argv[1][0] == '-')
  {
    usage(argv[0]);
    exit(1);
  }

  KXSConfigDialog *dialog = new KXSConfigDialog(argv[1]);
  dialog->show();

  app.setMainWidget(dialog);

  app.exec();

  delete dialog;
}

