/*
  Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  $Id$

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qlayout.h>
#include <qvgroupbox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kspell.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "spellchecking.h"

typedef KGenericFactory<KSpellCheckingConfig, QWidget > SpellFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_spellchecking, SpellFactory("kcmspellchecking") );


KSpellCheckingConfig::KSpellCheckingConfig(QWidget *parent, const char *name, const QStringList &):
    KCModule(parent, name)
{
  QBoxLayout *layout = new QVBoxLayout(this,
				       KDialog::marginHint(),
				       KDialog::spacingHint());
  QGroupBox *box = new QVGroupBox( i18n("Spell Checking Settings"), this );
  box->layout()->setSpacing( KDialog::spacingHint() );
  layout->addWidget(box);

  spellConfig = new KSpellConfig(box, 0L ,0L, false );
  layout->addStretch(1);
  connect(spellConfig,SIGNAL(configChanged()),this,SLOT(configChanged()));
}

void KSpellCheckingConfig::load()
{
}

void KSpellCheckingConfig::save()
{
    spellConfig->writeGlobalSettings();
}

void KSpellCheckingConfig::defaults()
{
    spellConfig->setNoRootAffix(0);
    spellConfig->setRunTogether(0);
    spellConfig->setDictionary("");
    spellConfig->setDictFromList(FALSE);
    spellConfig->setEncoding (KS_E_ASCII);
    spellConfig->setClient (KS_CLIENT_ISPELL);
}

QString KSpellCheckingConfig::quickHelp() const
{
  return i18n("<h1>Spell Checking</h1><p>This control module allows you to configure the KDE spell checking system. You can configure:<ul><li> which spell checking program to use<li> which types of spelling errors are identified<li> which dictionary is used by default.</ul><br>The KDE spell checking system (KSpell) provides support for two common spell checking utilities: ASpell and ISpell. This allows you to share dictionaries between KDE applications and non-KDE applications.</p>"); }

#include "spellchecking.moc"
