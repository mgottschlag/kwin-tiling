// Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>

#ifndef KWINTHEME_H
#define KWINTHEME_H

#include <kcmodule.h>

class QListBox;

class KWinThemeModule : public KCModule
{
  Q_OBJECT

  public:

    KWinThemeModule(QWidget * parent, const char * name);

    ~KWinThemeModule();

    virtual void load();
    virtual void save();
    virtual void defaults();

    void updateValues();
    void readSettings();

    QString quickHelp() const;

  signals:

    void changed(bool);

  protected slots:

    void slotSelectionChanged();
    void slotEnable(bool);

  private:

    QString savedLibraryName() const;
    QString selectedLibraryName() const;

    void resetKWin() const;

    QListBox * lb_themes_;

    class ThemeInfo
    {
      public:

        ThemeInfo()
        {
          // Empty.
        }

        ThemeInfo(QString name, QString libraryName)
          : name_         (name),
            libraryName_  (libraryName)
        {
          // Empty.
        }

        QString name() const { return name_; }
        QString libraryName() const { return libraryName_; }

      private:

        QString name_;
        QString libraryName_;
    };

    QValueList<ThemeInfo> themeInfoList_;
};

#endif

