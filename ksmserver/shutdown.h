/*****************************************************************
ksmserver - the KDE session management server
								  
Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <qdialog.h>
class QCheckBox;

class KSMShutdown : public QDialog
{
    Q_OBJECT
public:
    KSMShutdown();
    ~KSMShutdown();

    static bool shutdown( bool& saveSession );

private slots:
    void requestFocus();

private:
    void mousePressEvent( QMouseEvent * ){}
    QCheckBox* checkbox;

};

#endif
