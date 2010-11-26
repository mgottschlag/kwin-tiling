#include <kdebug.h>
#include <qtest_kde.h>
#include <QtGui/QIcon>

#include "../flags.h"
#include "../xkb_rules.h"
#include "../keyboard_config.h"


static QImage image(const QIcon& icon) {
	return icon.pixmap(QSize(16,16), QIcon::Normal, QIcon::On).toImage();
}

class FlagsTest : public QObject
{
    Q_OBJECT

	Flags* flags;
    const Rules* rules;

private Q_SLOTS:
    void initTestCase() {
    	flags = new Flags();
    	rules = NULL;
    }

    void cleanupTestCase() {
    	delete flags;
    	delete rules;
    }

    void testRules() {
        QVERIFY( flags != NULL );

        QVERIFY( ! flags->getTransparentPixmap().isNull() );

        const QIcon iconUs(flags->getIcon("us"));
        QVERIFY( ! iconUs.isNull() );
        QVERIFY( flags->getIcon("--").isNull() );

    	KeyboardConfig keyboardConfig;
        LayoutUnit layoutUnit("us");
        LayoutUnit layoutUnit1("us", "intl");
        layoutUnit1.setDisplayName("usi");
        LayoutUnit layoutUnit2("us", "other");

        keyboardConfig.showFlag = true;
        const QIcon iconUsFlag = flags->getIconWithText(layoutUnit, keyboardConfig);
        QVERIFY( ! iconUsFlag.isNull() );
        QCOMPARE( image(iconUsFlag), image(iconUs) );

        keyboardConfig.showFlag = false;
        const QIcon iconUsText = flags->getIconWithText(layoutUnit, keyboardConfig);
        QVERIFY( ! iconUsText.isNull() );
        QVERIFY( image(iconUsText) != image(iconUs) );

        keyboardConfig.layouts.append(layoutUnit1);
        QCOMPARE( flags->getShortText(layoutUnit, keyboardConfig), QString("us") );
        QCOMPARE( flags->getShortText(layoutUnit1, keyboardConfig), QString("usi") );
        QCOMPARE( flags->getShortText(layoutUnit2, keyboardConfig), QString("us") );

        const Rules* rules = Rules::readRules();
        QCOMPARE( flags->getLongText(layoutUnit, rules), QString("USA") );
        QVERIFY( flags->getLongText(layoutUnit1, rules).startsWith("USA - International") );
        QCOMPARE( flags->getLongText(layoutUnit2, rules), QString("USA - other") );

        flags->clearCache();
    }

//    void loadRulesBenchmark() {
//    	QBENCHMARK {
//    		Flags* flags = new Flags();
//    		delete flags;
//    	}
//    }

};

// need GUI for xkb protocol in xkb_rules.cpp
QTEST_KDEMAIN( FlagsTest, GUI )

#include "flags_test.moc"
