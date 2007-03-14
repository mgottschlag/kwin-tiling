#include "adddialog.h"

AddDialog::AddDialog (QWidget* parent) : QDialog( parent ) {
	widget = new Ui_AddDialog();
	widget->setupUi(this);

	connect( widget->btnImport, SIGNAL(clicked()), SLOT(importPrg()) );
	connect( widget->btnAdd, SIGNAL(clicked()), SLOT(addPrg()) );
	connect( widget->btnCancel, SIGNAL(clicked()), SLOT(reject()) );
}

AddDialog::~AddDialog()
{}

void AddDialog::importPrg() {
	done(3);
}
void AddDialog::addPrg() {
	done(4);
}

#include "adddialog.moc"
