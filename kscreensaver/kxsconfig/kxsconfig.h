
#ifndef __KXSCONFIG_H__
#define __KXSCONFIG_H__

#include <kdialogbase.h>
#include "kxsitem.h"

class KProcess;
class QLabel;

class KXSConfigDialog : public KDialogBase
{
  Q_OBJECT
public:
  KXSConfigDialog(const QString &file);
  ~KXSConfigDialog();

  QString command();

protected slots:
  void slotPreviewExited(KProcess *);
  void slotNewPreview();
  void slotChanged();
  virtual void slotOk();
  virtual void slotCancel();

protected:
  QString   mFilename;
  QString   mConfigFile;
  KProcess  *mPreviewProc;
  QWidget   *mPreview;
  QTimer    *mPreviewTimer;
  QList<KXSConfigItem> mConfigItemList;
};

#endif
