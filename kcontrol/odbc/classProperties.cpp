#include "classProperties.h"
#include "classProperties.moc"

classProperties::classProperties( QWidget* parent, const char* name, HODBCINSTPROPERTY hTheFirstProperty )
	: QDialog( parent, name, true )
{
    HODBCINSTPROPERTY 	hProperty;
	int				nProperty;
	pTopLayout		= new QVBoxLayout( this );
	QBoxLayout 		*pButtonLayout	= new QHBoxLayout();
	QPushButton 	*pbOk;
	QPushButton 	*pbCancel;

    hFirstProperty = hTheFirstProperty;

	/* PROPERTIES */
	for ( nProperties = 0, hProperty = hFirstProperty; hProperty != NULL; hProperty = hProperty->pNext )
	{
		nProperties++;
	}

	pGridLayout = new QGridLayout( nProperties, 2, 2 );
	pTopLayout->addLayout( pGridLayout );
	pGridLayout->setColStretch ( 0, 0 );
	pGridLayout->setColStretch ( 1, 1 );

	for ( nProperty = 0, hProperty = hFirstProperty; hProperty != NULL; nProperty++, hProperty = hProperty->pNext )
	{
		// 1ST COLUMN IS ALWAYS A LABEL CONTAINING THE PROPERTY NAME
        QLabel *pLabel = new QLabel( this );
		pLabel->setFrameStyle( QFrame::Box | QFrame::Sunken );
		pLabel->setLineWidth( 1 );
        pLabel->setText( hProperty->szName );
        pLabel->setMinimumSize( pLabel->sizeHint() );
		pLabel->setFixedHeight( pLabel->sizeHint().height() );
        pGridLayout->addWidget( pLabel, nProperty, 0 );

		// 2ND COLUMN IS WHERE THE USER ENTERS DATA SO CREATE A WIDGET THAT IS MEANINGFULL
		switch ( hProperty->nPromptType )
		{
		case ODBCINST_PROMPTTYPE_LABEL:
			{
				QLabel *pLabel2 = new QLabel( this );
				pLabel2->setFrameStyle( QFrame::Box | QFrame::Sunken );
				pLabel2->setLineWidth( 1 );
				pLabel2->setText( hProperty->szValue );
				pLabel2->setMinimumSize( pLabel2->sizeHint() );
				pLabel2->setFixedHeight( pLabel2->sizeHint().height() );
				pGridLayout->addWidget( pLabel2, nProperty, 1 );
                hProperty->pWidget = pLabel2;
                if ( hProperty->pszHelp ) QToolTip::add( pLabel2, hProperty->pszHelp );
			}
			break;
		case ODBCINST_PROMPTTYPE_LISTBOX:
			{
				QComboBox *pComboBox = new QComboBox( this );
				pComboBox->insertStrList( (const char **)hProperty->aPromptData );
				pComboBox->setMinimumSize( pComboBox->sizeHint() );
				pComboBox->setFixedHeight( pComboBox->sizeHint().height() );
				pGridLayout->addWidget( pComboBox, nProperty, 1 );
                hProperty->pWidget = pComboBox;
                if ( hProperty->pszHelp ) QToolTip::add( pComboBox, hProperty->pszHelp );
                setCurrentItem( pComboBox, hProperty->szValue );
            }
			break;
		case ODBCINST_PROMPTTYPE_COMBOBOX:
			{
				QComboBox *pComboBox = new QComboBox( true, this );
				pComboBox->insertStrList( (const char **)hProperty->aPromptData );
                pComboBox->setEditText( hProperty->szValue );
				pComboBox->setMinimumSize( pComboBox->sizeHint() );
				pComboBox->setFixedHeight( pComboBox->sizeHint().height() );
				pGridLayout->addWidget( pComboBox, nProperty, 1 );
                hProperty->pWidget = pComboBox;
                if ( hProperty->pszHelp ) QToolTip::add( pComboBox, hProperty->pszHelp );
			}
			break;
		case ODBCINST_PROMPTTYPE_FILENAME:
			{
                classFileSelector *pFileSelector = new classFileSelector( this );
				pFileSelector->pLineEdit->setText( hProperty->szValue );
				pGridLayout->addWidget( pFileSelector, nProperty, 1 );
                hProperty->pWidget = pFileSelector;
                if ( hProperty->pszHelp ) QToolTip::add( pFileSelector, hProperty->pszHelp );
			}
			break;
		default: // PROMPTTYPE_TEXTEDIT
			{
				QLineEdit *pLineEdit = new QLineEdit( this );
				pLineEdit->setText( hProperty->szValue );
				pLineEdit->setMinimumHeight( pLineEdit->sizeHint().height() );
				pLineEdit->setFixedHeight( pLineEdit->sizeHint().height() );
				pGridLayout->addWidget( pLineEdit, nProperty, 1 );
				pLabel->setBuddy( pLineEdit );
                hProperty->pWidget = pLineEdit;
                if ( hProperty->pszHelp ) QToolTip::add( pLineEdit, hProperty->pszHelp );
			}
		}
	}
	/* SPACER */
	QLabel *pSpacer = new QLabel( this );
	pTopLayout->addWidget( pSpacer, 11 );	

	/* OK CANCEL BUTTONS */
	pTopLayout->addLayout( pButtonLayout );
	
	pbOk = new QPushButton( this );
	pbOk->setText( "&Ok" );
	pbOk->setMinimumSize( pbOk->sizeHint() );
	pbOk->setFixedHeight( pbOk->sizeHint().height() );
	pButtonLayout->addWidget( pbOk );	

	pbCancel = new QPushButton( this );
	pbCancel->setText( "&Cancel" );
	pbCancel->setMinimumSize( pbCancel->sizeHint() );
	pbCancel->setFixedHeight( pbCancel->sizeHint().height() );
	pButtonLayout->addWidget( pbCancel );	

	// STATUS BAR
	QLabel *pStatus = new QLabel( this );
	pStatus->setText( "Ready" );
	pStatus->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	pStatus->setFixedHeight( pStatus->sizeHint().height() );
	pTopLayout->addWidget( pStatus );	


	pTopLayout->activate();

	connect( pbOk, SIGNAL(clicked()), this, SLOT(pbOk_Clicked()) );
	connect( pbCancel, SIGNAL(clicked()), this, SLOT(pbCancel_Clicked()) );
}


classProperties::~classProperties()
{
}

/*
void resizeEvent( QResizeEvent *p )
{
	pTopLayout->resize( p->size() );
}
*/

void classProperties::pbOk_Clicked()
{
    HODBCINSTPROPERTY 	hProperty;

	// COLLECT ALL VALUES
	for ( hProperty = hFirstProperty; hProperty != NULL; hProperty = hProperty->pNext )
	{
		switch ( hProperty->nPromptType )
		{
		case ODBCINST_PROMPTTYPE_LABEL:
			{
				strncpy( hProperty->szValue, ((QLabel *)hProperty->pWidget)->text(), INI_MAX_PROPERTY_VALUE );
			}
			break;
		case ODBCINST_PROMPTTYPE_LISTBOX:
		case ODBCINST_PROMPTTYPE_COMBOBOX:
			{
				strncpy( hProperty->szValue, ((QComboBox *)hProperty->pWidget)->currentText(), INI_MAX_PROPERTY_VALUE );
			}
			break;
		case ODBCINST_PROMPTTYPE_FILENAME:
			{
				strncpy( hProperty->szValue, ((classFileSelector *)hProperty->pWidget)->pLineEdit->text(), INI_MAX_PROPERTY_VALUE );
			}
			break;
		default: // PROMPTTYPE_TEXTEDIT
			{
				strncpy( hProperty->szValue, ((QLineEdit *)hProperty->pWidget)->text(), INI_MAX_PROPERTY_VALUE );
			}
		}
	} // for

	// LET CALLER KNOW WHAT TO DO NEXT
	accept();
}

void classProperties::pbCancel_Clicked()
{
	reject();
}

void classProperties::setCurrentItem( QComboBox *pComboBox, char *pszItem )
{
	int n 			= 0;
	int nCurItem 	= 0;

	nCurItem = pComboBox->currentItem();
	for ( n=0; n < pComboBox->count(); n++ )
	{
		pComboBox->setCurrentItem( n );
		if ( strcasecmp( pszItem, pComboBox->currentText().data() ) == 0 )
		{
			return;
		}
	}

	pComboBox->setCurrentItem( nCurItem );
}

