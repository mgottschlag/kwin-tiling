/*
 *  Copyright (C) 2011 Andriy Rysin (rysin@kde.org)
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

#include <kdebug.h>
#include <QtGui/QApplication>
#include <qtest_kde.h>

#include "../xkb_rules.h"


class RulesTest : public QObject
{
    Q_OBJECT

	Rules* rules;

private Q_SLOTS:
    void initTestCase() {
    	rules = Rules::readRules();
    }

    void cleanupTestCase() {
    	delete rules;
    }

    void testRules() {
        QVERIFY( rules != NULL );
        QVERIFY( rules->modelInfos.size() > 0 );
        QVERIFY( rules->layoutInfos.size() > 0 );
        QVERIFY( rules->optionGroupInfos.size() > 0 );
    }

    void testModel() {
        foreach(const ModelInfo* modelInfo, rules->modelInfos) {
        	QVERIFY( modelInfo != NULL);
        	QVERIFY( modelInfo->name.length() > 0 );
        	QVERIFY( modelInfo->description.length() > 0 );
//        	QVERIFY( ! modelInfo->vendor.isEmpty() );
        }
    }

    void testLayouts() {
        foreach(const LayoutInfo* layoutInfo, rules->layoutInfos) {
        	QVERIFY( layoutInfo != NULL);
        	QVERIFY( ! layoutInfo->name.isEmpty() );
//        	const char* desc = layoutInfo->name.toUtf8() ;
//        	qDebug() << layoutInfo->name;
        	QVERIFY( ! layoutInfo->description.isEmpty() );

        	foreach(const VariantInfo* variantInfo, layoutInfo->variantInfos) {
        		QVERIFY( variantInfo != NULL );
        		QVERIFY( ! variantInfo->name.isEmpty() );
        		QVERIFY( ! variantInfo->description.isEmpty() );
        	}
        	foreach(const QString& language, layoutInfo->languages) {
        		QVERIFY( ! language.isEmpty() );
        	}
        }
    }

    void testOptionGroups() {
        foreach(const OptionGroupInfo* optionGroupInfo, rules->optionGroupInfos) {
        	QVERIFY( optionGroupInfo != NULL);
        	QVERIFY( ! optionGroupInfo->name.isEmpty() );
        	QVERIFY( ! optionGroupInfo->description.isEmpty() );
        	// optionGroupInfo->exclusive

        	foreach(const OptionInfo* optionInfo, optionGroupInfo->optionInfos) {
        		QVERIFY( optionInfo != NULL );
        		QVERIFY( ! optionInfo->name.isEmpty() );
        		QVERIFY( ! optionInfo->description.isEmpty() );
        	}
        }
    }

    void loadRulesBenchmark() {
    	QBENCHMARK {
    		Rules* rules = Rules::readRules();
    		delete rules;
    	}
    }

};

// need kde libs for config-workspace.h used in xkb_rules.cpp
// need GUI for xkb protocol
QTEST_KDEMAIN( RulesTest, GUI )

#include "xkb_rules_test.moc"
