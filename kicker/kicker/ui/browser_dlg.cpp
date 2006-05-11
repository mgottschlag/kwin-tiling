/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <QLabel>
#include <QLayout>

#include <kglobal.h>
#include <klocale.h>
#include <kicondialog.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include "browser_dlg.h"
#include "browser_dlg.moc"

PanelBrowserDialog::PanelBrowserDialog( const QString& path, const QString &icon, QWidget *parent, const char *name )
    :  KDialogBase( parent, name, true, i18n( "Quick Browser Configuration" ), Ok|Cancel, Ok, true )
{
    setMinimumWidth( 300 );

    KVBox *page = makeVBoxMainWidget();

    QWidget* widget = new QWidget(page);
    QHBoxLayout *hbox = new QHBoxLayout(widget);
    hbox->setSpacing( KDialog::spacingHint() );
    QLabel *label1 = new QLabel( i18n( "Button icon:" ), widget );
    hbox->addWidget(label1);

    iconBtn = new KIconButton(page);
    hbox->addWidget(iconBtn);
    iconBtn->setFixedSize( 50, 50 );
    iconBtn->setIconType( K3Icon::Panel, K3Icon::FileSystem );
    label1->setBuddy( iconBtn );

    widget = new QWidget(page);
    hbox = new QHBoxLayout(widget);
    hbox->setSpacing(KDialog::spacingHint());
    QLabel *label2 = new QLabel(i18n("Path:"), widget);
    hbox->addWidget(label2);
    pathInput = new KLineEdit(widget);
    pathInput->setText(path);
    pathInput->setFocus();
    hbox->addWidget(pathInput);
    connect(pathInput, SIGNAL( textChanged ( const QString & )), this, SLOT( slotPathChanged( const QString & )));

    label2->setBuddy( pathInput );
    browseBtn = new QPushButton(i18n("&Browse..."), widget);
    hbox->addWidget(browseBtn);

    if (icon.isEmpty())
    {
        KUrl u;
        u.setPath(path);
        iconBtn->setIcon(KMimeType::iconNameForURL(u));
    }
    else
    {
        iconBtn->setIcon(icon);
    }

    connect( browseBtn, SIGNAL( clicked() ), this, SLOT( browse() ) );
}

PanelBrowserDialog::~PanelBrowserDialog()
{

}

void PanelBrowserDialog::slotPathChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

void PanelBrowserDialog::browse()
{
    QString dir = KFileDialog::getExistingDirectory( pathInput->text(), 0, i18n( "Select Folder" ) );
    if ( !dir.isEmpty() ) {
        pathInput->setText( dir );
        KUrl u;
        u.setPath( dir );
        iconBtn->setIcon( KMimeType::iconNameForURL( u ) );
    }
}

void PanelBrowserDialog::slotOk()
{
    QDir dir(path());
    if( !dir.exists() ) {
        KMessageBox::sorry( this, i18n("'%1' is not a valid folder.", path()) );
        return;
    }
    KDialogBase::slotOk();
}

const QString PanelBrowserDialog::icon()
{
    return iconBtn->icon();
}

QString PanelBrowserDialog::path()
{
    return pathInput->text();
}
