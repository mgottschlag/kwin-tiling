/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcm_add_layout_dialog.h"

#include <klocalizedstring.h>

#include <QtCore/QDebug>

#include "xkb_rules.h"
#include "flags.h"
#include "iso_codes.h"

#include "ui_kcm_add_layout_dialog.h"


AddLayoutDialog::AddLayoutDialog(const Rules* rules_, Flags* flags_, QWidget* parent):
		QDialog(parent),
		rules(rules_),
		flags(flags_)
{
    layoutDialogUi = new Ui_AddLayoutDialog();
    layoutDialogUi->setupUi(this);

    QSet<QString> languages;
    foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
    	QSet<QString> langs = QSet<QString>::fromList(layoutInfo->languages);
    	languages.unite( langs );
    }
    IsoCodes isoCodes(IsoCodes::iso_639);
    foreach(QString lang, languages) {
    	const IsoCodeEntry* isoCodeEntry = isoCodes.getEntry(IsoCodes::attr_iso_639_2B_code, lang);
    	if( isoCodeEntry == NULL ) {
    		isoCodeEntry = isoCodes.getEntry(IsoCodes::attr_iso_639_2T_code, lang);
    	}
    	QString name = isoCodeEntry != NULL ? i18n(isoCodeEntry->value(IsoCodes::attr_name).toUtf8()) : lang;
    	layoutDialogUi->languageComboBox->addItem(name, lang);
    }
    layoutDialogUi->languageComboBox->model()->sort(0);
	layoutDialogUi->languageComboBox->insertItem(0, i18n("Any language"), "");
	layoutDialogUi->languageComboBox->setCurrentIndex(0);
    languageChanged(0);

	connect(layoutDialogUi->languageComboBox, SIGNAL(activated(int)), this, SLOT(languageChanged(int)));
    connect(layoutDialogUi->layoutComboBox, SIGNAL(activated(int)), this, SLOT(layoutChanged(int)));
}

void AddLayoutDialog::languageChanged(int langIdx)
{
	QString lang = layoutDialogUi->languageComboBox->itemData(langIdx).toString();
	qDebug() << "selected lang" << lang;

	QPixmap emptyPixmap(layoutDialogUi->layoutComboBox->iconSize());
	emptyPixmap.fill(Qt::transparent);

	layoutDialogUi->layoutComboBox->clear();
    foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
    	if( lang.isEmpty() || layoutInfo->languages.contains(lang) ) {
    		QIcon icon(flags->getIcon(layoutInfo->name));
    		if( icon.isNull() ) {
    			icon = QIcon(emptyPixmap);	// align text with no icons
    		}
    		layoutDialogUi->layoutComboBox->addItem(icon, layoutInfo->description, layoutInfo->name);
    	}
    }
	layoutDialogUi->layoutComboBox->setCurrentIndex(0);
	layoutChanged(0);
}

void AddLayoutDialog::layoutChanged(int layoutIdx)
{
	layoutDialogUi->variantComboBox->clear();
	const LayoutInfo* layoutInfo = rules->layoutInfos[layoutIdx];
    foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
    	layoutDialogUi->variantComboBox->addItem(variantInfo->description, variantInfo->name);
    }
    layoutDialogUi->variantComboBox->model()->sort(0);
	layoutDialogUi->variantComboBox->insertItem(0, i18nc("variant", "Default"), "");
	layoutDialogUi->variantComboBox->setCurrentIndex(0);
}

void AddLayoutDialog::accept()
{
	selectedLayoutConfig.layout = layoutDialogUi->layoutComboBox->itemData(layoutDialogUi->layoutComboBox->currentIndex()).toString();
	selectedLayoutConfig.variant = layoutDialogUi->variantComboBox->itemData(layoutDialogUi->variantComboBox->currentIndex()).toString();;
	QDialog::accept();
}