#include <QString>
#include <QtTest>
#include <QCoreApplication>

class vmtestDocumentEditorTest : public QObject
{
	Q_OBJECT

public:
	vmtestDocumentEditorTest();

private Q_SLOTS:
	void testCase1();
};

vmtestDocumentEditorTest::vmtestDocumentEditorTest()
{
}

void vmtestDocumentEditorTest::testCase1()
{
	QVERIFY2(true, "Failure");
}

QTEST_MAIN(vmtestDocumentEditorTest)

#include "tst_vmtestdocumenteditortest.moc"
