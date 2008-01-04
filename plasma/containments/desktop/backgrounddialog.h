/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef BACKGROUNDDIALOG_H
#define BACKGROUNDDIALOG_H

#include <QSize>
#include <QTimer>
#include <KDialog>

#include "backgroundpackage.h"
#include "renderthread.h"

class BackgroundContainer;
class BackgroundListModel;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QTimeEdit;
class QCheckBox;
class KColorButton;
class KSeparator;

class BackgroundDialog : public KDialog
{
Q_OBJECT
public:
    enum BackgroundMode {
        kStaticBackground,
        kSlideshowBackground
    };
    
    // FIXME seems that we're leaking, make a distructor
    BackgroundDialog(const QSize &res, 
                     const KConfigGroup &config,
                     QWidget *parent = 0);
    
    void reloadConfig(const KConfigGroup &config);
    void saveConfig(KConfigGroup config);
    
    QString path() const;
    int mode();
private:
    QComboBox *m_mode;
    QComboBox *m_view;
    BackgroundListModel *m_model;
    
    QLabel *m_authorLabel;
    QLabel *m_emailLabel;
    QLabel *m_licenseLabel;
    QLabel *m_authorLine;
    QLabel *m_emailLine;
    QLabel *m_licenseLine;
    QLabel *m_preview;
    
    QPushButton *m_newStuff;
    
    QComboBox *m_resizeMethod;
    KColorButton *m_color;
    
    QListWidget *m_dirlist;
    QPushButton *m_addDir;
    QPushButton *m_removeDir;
    QTimeEdit *m_slideshowDelay;
    
    QString m_img;
    QSize m_res;
    float m_ratio;
    
    QTimer m_preview_timer;
    QList<Background *> m_slideshowBackgrounds;
    int m_currentSlide;
    
    QStringList m_selected;
    
    RenderThread m_preview_renderer;
    int m_preview_token;

    QCheckBox *m_alignToGrid;
    QCheckBox *m_showIcons;
    
    bool setMetadata(QLabel *label,
                     const QString &text);
    void setPreview(const QString &img, Background::ResizeMethod method);
    virtual bool contains(const QString &path) const;
private slots:
    void update();
    void getNewStuff();
    void browse();
    
    void slotAddDir();
    void slotRemoveDir();
    void updateSlideshow();
    void updateSlideshowPreview();
    
    void changeBackgroundMode(int mode);
    void previewRenderingDone(int token, const QImage &pix);
    
    void updateScreenshot(QPersistentModelIndex index);
    void cleanup();
};

#endif // BACKGROUNDDIALOG_H
