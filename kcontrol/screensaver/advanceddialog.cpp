#include <klocale.h>

#include "advanceddialog.h"

KScreenSaverAdvancedDialog::KScreenSaverAdvancedDialog(QWidget *parent, const char* name)
 : KDialogBase( parent, name, true, i18n( "Advanced Options" ),
                Ok | Cancel, Ok, true )
{
    readSettings();

    QWidget* w = new QWidget( this );
    w->setMinimumWidth( 250 );
    QBoxLayout* mainLayout = new QVBoxLayout( w, 0, KDialog::spacingHint() );
    setMainWidget( w );

    // Autolock
    QGroupBox* autoLockGroup = new QGroupBox( i18n("Auto Lock"), w );
    autoLockGroup->setColumnLayout( 0, Qt::Vertical );
    mainLayout->addWidget( autoLockGroup );
    QBoxLayout* autoLockLayout = new QVBoxLayout( autoLockGroup->layout(),
        KDialog::spacingHint() );

    m_topLeftCorner = new QCheckBox( i18n("Top-left corner"), autoLockGroup);
    autoLockLayout->addWidget( m_topLeftCorner );
    m_topLeftCorner->setChecked(mTopLeftCorner);
    connect( m_topLeftCorner, SIGNAL( toggled( bool ) ),
            this, SLOT( slotChangeTopLeftCorner( bool ) ) );

    m_topRightCorner = new QCheckBox( i18n("Top-right corner"),
        autoLockGroup );
    autoLockLayout->addWidget( m_topRightCorner );
    m_topRightCorner->setChecked(mTopRightCorner);
    connect( m_topRightCorner, SIGNAL( toggled( bool ) ),
         this, SLOT( slotChangeTopRightCorner( bool ) ) );

    m_bottomLeftCorner = new QCheckBox( i18n("Bottom-left corner"),
        autoLockGroup );
    autoLockLayout->addWidget( m_bottomLeftCorner );
    m_bottomLeftCorner->setChecked(mBottomLeftCorner);
    connect( m_bottomLeftCorner, SIGNAL( toggled( bool ) ),
         this, SLOT( slotChangeBottomLeftCorner( bool ) ) );

    m_bottomRightCorner = new QCheckBox( i18n("Bottom-right corner"),
        autoLockGroup );
    autoLockLayout->addWidget( m_bottomRightCorner );
    m_bottomRightCorner->setChecked(mBottomRightCorner);
    connect( m_bottomRightCorner, SIGNAL( toggled( bool ) ),
         this, SLOT( slotChangeBottomRightCorner( bool ) ) );

    // Priority
    QGroupBox* priorityGroup = new QGroupBox( i18n("&Priority"), w );
    priorityGroup->setColumnLayout( 0, Qt::Horizontal );
    mainLayout->addWidget( priorityGroup );

    QGridLayout *gl = new QGridLayout(priorityGroup->layout(), 2, 3);
    gl->setColStretch( 1, 10 );

    mPrioritySlider = new QSlider(QSlider::Horizontal, priorityGroup);
    mPrioritySlider->setRange(0, 19);
    mPrioritySlider->setSteps(1, 5);
    mPrioritySlider->setTickmarks(QSlider::Below);
    mPrioritySlider->setValue(19 - mPriority);
    connect(mPrioritySlider, SIGNAL( valueChanged(int)),
        SLOT(slotPriorityChanged(int)));
    gl->addMultiCellWidget(mPrioritySlider, 0, 0, 0, 2);
    QWhatsThis::add( mPrioritySlider, i18n("Use this slider to change the"
      " processing priority for the screen saver over other jobs that are"
      " being executed in the background. For a processor-intensive screen"
      " saver, setting a higher priority may make the display smoother at"
      " the expense of other jobs.") );

#ifndef HAVE_SETPRIORITY
    mPrioritySlider->setEnabled(false);
#endif

    QLabel* lbl = new QLabel(i18n("Low Priority", "Low"), priorityGroup);
    gl->addWidget(lbl, 1, 0);

#ifndef HAVE_SETPRIORITY
    lbl->setEnabled(false);
#endif

    lbl = new QLabel(i18n("High Priority", "High"), priorityGroup);
    gl->addWidget(lbl, 1, 2);

#ifndef HAVE_SETPRIORITY
    lbl->setEnabled(false);
#endif

    mainLayout->addStretch(10);
}


void KScreenSaverAdvancedDialog::readSettings()
{
    KConfig *config = new KConfig( "kdesktoprc");
    config->setGroup( "ScreenSaver" );

    mPriority = config->readNumEntry("Priority", 19);
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;

    mTopLeftCorner = config->readBoolEntry("LockCornerTopLeft", false);
    mTopRightCorner = config->readBoolEntry("LockCornerTopRight", false) ;
    mBottomLeftCorner = config->readBoolEntry("LockCornerBottomLeft", false);
    mBottomRightCorner = config->readBoolEntry("LockCornerBottomRight", false);
    mChanged = false;
    delete config;
}

void KScreenSaverAdvancedDialog::slotPriorityChanged( int val )
{
    if (val == mPriority)
    return;

    mPriority = 19 - val;
    if (mPriority > 19)
    mPriority = 19;
    else if (mPriority < 0)
    mPriority = 0;

    mChanged = true;
}

void KScreenSaverAdvancedDialog::slotOk()
{
    if ( mChanged ) {
        KConfig *config = new KConfig( "kdesktoprc");
        config->setGroup( "ScreenSaver" );

        config->writeEntry("Priority", mPriority);
        config->writeEntry(
            "LockCornerTopLeft", m_topLeftCorner->isChecked());
        config->writeEntry(
            "LockCornerBottomLeft", m_bottomLeftCorner->isChecked());
        config->writeEntry(
            "LockCornerTopRight", m_topRightCorner->isChecked());
        config->writeEntry(
            "LockCornerBottomRight", m_bottomRightCorner->isChecked());
        config->sync();
        delete config;
    }
    accept();
}

void KScreenSaverAdvancedDialog::slotChangeBottomRightCorner( bool b)
{
    mBottomRightCorner = b;
    mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeBottomLeftCorner( bool b)
{
    mBottomLeftCorner = b;
    mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeTopRightCorner( bool b)
{
    mTopRightCorner = b;
    mChanged = true;
}

void KScreenSaverAdvancedDialog::slotChangeTopLeftCorner( bool b)
{
    mTopLeftCorner = b;
    mChanged = true;
}
