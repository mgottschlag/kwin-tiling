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

#include "rules_test.moc"
