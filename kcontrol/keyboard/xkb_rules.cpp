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

#include "xkb_rules.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QtCore/QDir>
#include <QtGui/QTextDocument> // for Qt::escape
#include <QtXml/QXmlAttributes>

//#include <libintl.h>
//#include <locale.h>

#include "x11_helper.h"

// for findXkbRuleFile
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <config-workspace.h>


class RulesHandler : public QXmlDefaultHandler
{
public:
	RulesHandler(Rules* rules_):
		rules(rules_) {}

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);
//    bool fatalError(const QXmlParseException &exception);
//    QString errorString() const;

private:
//    QString getString(const QString& text);

    QStringList path;
    Rules* rules;
};

static QString translate_xml_item(const QString& itemText)
{
	return i18n(Qt::escape(itemText).toUtf8());
	//	return QString::fromUtf8(dgettext("xkeyboard-config", itemText.toAscii()));
}

static QString translate_description(ConfigItem* item)
{
	return item->description.isEmpty()
			? item->name : translate_xml_item(item->description);
}

#define removeEmptyItems(list) ;

//TODO: this is tricky: we want a nice API but we don't want 6 functions to be created :(
//template<class T>
//void removeEmptyItems(QList<T*>& list)
//{
//	QList<T*>::iterator i;
//	for (i = list.begin(); i != list.end(); ++i) {
//		if( static_cast<ConfigItem*>(*i)->name.isEmpty() ) {
//			list.erase(i);
//		}
//	}
//}

void postProcess(Rules* rules)
{
	//TODO remove elements with empty names to safeguard us
	removeEmptyItems(rules->layoutInfos);
	removeEmptyItems(rules->modelInfos);
	removeEmptyItems(rules->optionGroupInfos);

	KGlobal::locale()->insertCatalog("xkeyboard-config");
//	setlocale(LC_ALL, "");
//	bindtextdomain("xkeyboard-config", LOCALE_DIR);
	foreach(ModelInfo* modelInfo, rules->modelInfos) {
		modelInfo->vendor = translate_xml_item(modelInfo->vendor);
		modelInfo->description = translate_description(modelInfo);
	}
	foreach(LayoutInfo* layoutInfo, rules->layoutInfos) {
		layoutInfo->description = translate_description(layoutInfo);

		removeEmptyItems(layoutInfo->variantInfos);
		foreach(VariantInfo* variantInfo, layoutInfo->variantInfos) {
			variantInfo->description = translate_description(variantInfo);
		}
	}
	foreach(OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
		optionGroupInfo->description = translate_description(optionGroupInfo);

		removeEmptyItems(optionGroupInfo->optionInfos);
		foreach(OptionInfo* optionInfo, optionGroupInfo->optionInfos) {
			optionInfo->description = translate_description(optionInfo);
		}
	}
	KGlobal::locale()->removeCatalog("xkeyboard-config");
}


Rules::Rules():
	version("1.0")
{
}

QString Rules::getRulesName()
{
	XkbRF_VarDefsRec vd;
	char *tmp = NULL;

	if (XkbRF_GetNamesProp(QX11Info::display(), &tmp, &vd) && tmp != NULL ) {
		// 			qDebug() << "namesprop" << tmp ;
		return QString(tmp);
	}

	return QString::null;
}

static QString findXkbRulesFile()
{
	QString rulesFile;
	QString rulesName = Rules::getRulesName();

	if ( ! rulesName.isNull() ) {
		QString xkbParentDir;

		QString base(XLIBDIR);
		if( base.count('/') >= 3 ) {
			// .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
			QString delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
			QDir baseDir(base + delta);
			if( baseDir.exists() ) {
				xkbParentDir = baseDir.absolutePath();
			}
			else {
				QDir baseDir(base + "/X11");	// .../usr/X11/lib/X11/xkb (old XFree)
				if( baseDir.exists() ) {
					xkbParentDir = baseDir.absolutePath();
				}
			}
		}

		if( xkbParentDir.isEmpty() ) {
			xkbParentDir = "/usr/share/X11";
		}

		rulesFile = QString("%1/xkb/rules/%2.xml").arg(xkbParentDir, rulesName);
	}

	return rulesFile;
}


const char Rules::XKB_OPTION_GROUP_SEPARATOR = ':';

Rules* Rules::readRules()
{
	return readRules(findXkbRulesFile());
}

Rules* Rules::readRules(const QString& filename)
{
	QFile file(filename);
	if( !file.open(QFile::ReadOnly | QFile::Text) ) {
		qWarning() << "Cannot open the rules file" << file.fileName();
		return NULL;
	}

	Rules* rules = new Rules();
	RulesHandler rulesHandler(rules);

	QXmlSimpleReader reader;
	reader.setContentHandler(&rulesHandler);
	reader.setErrorHandler(&rulesHandler);

	QXmlInputSource xmlInputSource(&file);

	kDebug() << "Parsing xkb rules from" << file.fileName();

	if( ! reader.parse(xmlInputSource) ) {
		qWarning() << "Failed to parse the rules file" << file.fileName();
		delete rules;
		return NULL;
	}

	postProcess(rules);

	return rules;
}

bool RulesHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/,
                      const QString &qName, const QXmlAttributes &attributes)
{
	path << QString(qName);

	QString strPath = path.join("/");
	if( strPath.endsWith("layoutList/layout/configItem") ) {
		rules->layoutInfos << new LayoutInfo();
	}
	else if( strPath.endsWith("layoutList/layout/variantList/variant") ) {
		rules->layoutInfos.last()->variantInfos << new VariantInfo();
	}
	else if( strPath.endsWith("modelList/model") ) {
		rules->modelInfos << new ModelInfo();
	}
	else if( strPath.endsWith("optionList/group") ) {
		rules->optionGroupInfos << new OptionGroupInfo();
		rules->optionGroupInfos.last()->exclusive = (attributes.value("allowMultipleSelection") != "true");
	}
	else if( strPath.endsWith("optionList/group/option") ) {
		rules->optionGroupInfos.last()->optionInfos << new OptionInfo();
	}
	else if( strPath == ("xkbConfigRegistry") && ! attributes.value("version").isEmpty()  ) {
		rules->version = attributes.value("version");
		kDebug() << "xkbConfigRegistry version" << rules->version;
	}
	return true;
}

bool RulesHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &/*qName*/)
{
	path.removeLast();
	return true;
}

bool RulesHandler::characters(const QString &str)
{
	if( !str.trimmed().isEmpty() ) {
		QString strPath = path.join("/");
		if( strPath.endsWith("layoutList/layout/configItem/name") ) {
			if( rules->layoutInfos.last() != NULL ) {
				rules->layoutInfos.last()->name = str.trimmed();
//				qDebug() << "name:" << str;
			}
			// skipping invalid entry
		}
		else if( strPath.endsWith("layoutList/layout/configItem/description") ) {
			rules->layoutInfos.last()->description = str.trimmed();
//			qDebug() << "descr:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/configItem/languageList/iso639Id") ) {
			rules->layoutInfos.last()->languages << str.trimmed();
//			qDebug() << "\tlang:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/variantList/variant/configItem/name") ) {
			rules->layoutInfos.last()->variantInfos.last()->name = str.trimmed();
//			qDebug() << "\tvariant name:" << str;
		}
		else if( strPath.endsWith("layoutList/layout/variantList/variant/configItem/description") ) {
			rules->layoutInfos.last()->variantInfos.last()->description = str.trimmed();
//			qDebug() << "\tvariant descr:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/name") ) {
			rules->modelInfos.last()->name = str.trimmed();
//			qDebug() << "name:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/description") ) {
			rules->modelInfos.last()->description = str.trimmed();
//			qDebug() << "\tdescr:" << str;
		}
		else if( strPath.endsWith("modelList/model/configItem/vendor") ) {
			rules->modelInfos.last()->vendor = str.trimmed();
//			qDebug() << "\tvendor:" << str;
		}
		else if( strPath.endsWith("optionList/group/configItem/name") ) {
			rules->optionGroupInfos.last()->name = str.trimmed();
//			qDebug() << "name:" << str;
		}
		else if( strPath.endsWith("optionList/group/configItem/description") ) {
			rules->optionGroupInfos.last()->description = str.trimmed();
//			qDebug() << "\tdescr:" << str;
		}
		else if( strPath.endsWith("optionList/group/option/configItem/name") ) {
			rules->optionGroupInfos.last()->optionInfos.last()->name = str.trimmed();
//			qDebug() << "name:" << str;
		}
		else if( strPath.endsWith("optionList/group/option/configItem/description") ) {
			rules->optionGroupInfos.last()->optionInfos.last()->description = str.trimmed();
//			qDebug() << "\tdescr:" << str;
		}
	}
	return true;
}

