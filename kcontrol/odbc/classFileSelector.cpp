#include "classFileSelector.h"
#include "classFileSelector.moc"

classFileSelector::classFileSelector( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	QBoxLayout 		*pTopLayout		= new QHBoxLayout( this );

	pLineEdit 	= new QLineEdit( this );
	pButton		= new QPushButton( ">", this );
    pButton->resize( pButton->sizeHint() );
	pButton->setFixedWidth( pButton->sizeHint().width() );


    pTopLayout->addWidget( pLineEdit, 2 );
    pTopLayout->addWidget( pButton, 1 );

    pTopLayout->activate();

	connect( pButton, SIGNAL(clicked()), this, SLOT(pButton_Clicked()) );
}

classFileSelector::~classFileSelector()
{
}

void classFileSelector::pButton_Clicked()
{
	QString qsFile( QFileDialog::getOpenFileName() );
    if ( qsFile.isNull() )
	    return;

	pLineEdit->setText( qsFile );
}


