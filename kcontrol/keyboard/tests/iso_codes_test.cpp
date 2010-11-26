#include <kdebug.h>
#include <QtGui/QApplication>
#include <qtest_kde.h>

#include "../iso_codes.h"


class IsoCodesTest : public QObject
{
    Q_OBJECT

    IsoCodes* isoCodes;

private Q_SLOTS:
    void initTestCase() {
    	isoCodes = new IsoCodes(IsoCodes::iso_639);
    }

    void cleanupTestCase() {
    	delete isoCodes;
    }

    void testIsoCodes() {
        QVERIFY( isoCodes != NULL );
        QVERIFY( ! isoCodes->getEntryList().isEmpty() );
        const IsoCodeEntry* isoEntry = isoCodes->getEntry(IsoCodes::attr_iso_639_2T_code, "eng");
        QVERIFY( isoEntry != NULL );
        QVERIFY( ! isoEntry->empty() );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2T_code), QString("eng") );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_2B_code), QString("eng") );
        QCOMPARE( isoEntry->value(IsoCodes::attr_iso_639_1_code), QString("en") );
    }

    void loadIsoCodesBenchmark() {
    	QBENCHMARK {
    		IsoCodes* isoCodes = new IsoCodes(IsoCodes::iso_639);
    		delete isoCodes;
    	}
    }

};

//TODO: something lighter than KDEMAIN ?
QTEST_KDEMAIN( IsoCodesTest, NoGUI )

#include "iso_codes_test.moc"
