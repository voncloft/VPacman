#include "mainwindow.h"
#include <QtTest>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTemporaryDir>
#include <QLabel>
extern QString test;
class VpacmanTest : public QObject {
    Q_OBJECT
private slots:
    void bundledIconRendersWithoutTheme() {
        const QIcon icon(":/icons/vpacman.svg");
        QVERIFY(!icon.isNull());
        const QPixmap pixmap = icon.pixmap(64, 64);
        QVERIFY(!pixmap.isNull());
        QVERIFY(pixmap.toImage().pixelColor(32, 32).alpha() > 0);
    }
    void windowIconSurvivesClosingAndReopening() {
        for (int i = 0; i < 2; ++i) {
            MainWindow window;
            window.show();
            QVERIFY(!window.windowIcon().pixmap(32, 32).isNull());
            window.close();
        }
    }
    void repositoryAndSearch() {
        MainWindow w;
        auto *list=w.findChild<QListWidget *>("listWidget");
        auto *search=w.findChild<QPlainTextEdit *>("plainTextEdit");
        QVERIFY(list); QVERIFY(search); QVERIFY(list->count()>100);
        const auto processes=w.findChildren<QProcess *>();
        for (auto *p:processes) QCOMPARE(p->state(),QProcess::NotRunning);
        search->setPlainText("vPaCmAn");
        QVERIFY(list->count()>=1);
        bool found=false;
        for(int i=0;i<list->count();++i) {
            auto *item=list->item(i);
            if(item->data(Qt::UserRole).toString()=="vpacman") {
                found=true;
                QVERIFY(item->text().contains("[kde-apps] vpacman - "));
                QVERIFY(item->text().contains("Qt 6 graphical package manager"));
            }
        }
        QVERIFY(found);
        search->setPlainText("__nonexistent-package-for-test__");
        QCOMPARE(list->count(),0);
        QVERIFY(w.findChild<QTextEdit *>("textEdit")->isReadOnly());
    }
    void configuredRepositoriesAndReload() {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        auto write = [](const QString &path, const QByteArray &contents) {
            QFile file(path);
            QVERIFY(file.open(QIODevice::WriteOnly));
            QCOMPARE(file.write(contents), contents.size());
        };
        // All fixture repositories are outside the old hardcoded source tree.
        for (const QString &path : {QString("extra/fixture-extra"), QString("custom/fixture-custom"), QString("disabled/fixture-hidden")}) {
            QVERIFY(QDir().mkpath(tmp.filePath(path)));
            write(tmp.filePath(path + "/spkgbuild"), "# description : Fixture package\nname=fixture\nversion=1.2\n");
        }
        const QString config = tmp.filePath("scratchpkg.repo");
        const QByteArray enabled = ("  # comment\n\n" + tmp.filePath("extra") + "\thttps://example.invalid/extra\n"
            + "#" + tmp.filePath("disabled") + " https://example.invalid/disabled\n"
            + "  " + tmp.filePath("custom") + "  https://example.invalid/custom\n"
            + tmp.filePath("extra/") + " https://example.invalid/duplicate\n"
            + tmp.filePath("missing") + " https://example.invalid/missing\n").toUtf8();
        write(config, enabled);
        MainWindow window(nullptr, config);
        auto *list = window.findChild<QListWidget *>("listWidget");
        auto *search = window.findChild<QPlainTextEdit *>("plainTextEdit");
        QCOMPARE(list->count(), 2);
        QVERIFY(window.findChild<QLabel *>("label")->text().contains("Unavailable:"));
        search->setPlainText("[extra]");
        QCOMPARE(list->count(), 1);
        QVERIFY(list->item(0)->text().contains("fixture-extra - 1.2: Fixture package"));
        // Reload reads edits to the configuration, including newly enabled repos.
        write(config, (tmp.filePath("disabled") + " https://example.invalid/disabled\n").toUtf8());
        search->clear();
        QCOMPARE(list->count(), 1);
        QCOMPARE(list->item(0)->data(Qt::UserRole).toString(), QString("fixture-hidden"));
        QVERIFY(QFile::remove(config));
        QVERIFY(QMetaObject::invokeMethod(&window, "loadPackages"));
        QCOMPARE(list->count(), 0);
        QVERIFY(window.findChild<QLabel *>("label")->text().contains("Cannot read repositories:"));
    }
    void liveExtraRepositoryMatchesReadableRecipes() {
        const QDir extra("/sources/Voncloft-OS-dev/extra");
        int expected = 0;
        for (const auto &name : extra.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFile file(extra.filePath(name + "/spkgbuild"));
            if (file.open(QIODevice::ReadOnly)) ++expected;
        }
        MainWindow window;
        auto *list = window.findChild<QListWidget *>("listWidget");
        window.findChild<QPlainTextEdit *>("plainTextEdit")->setPlainText("[extra]");
        QVERIFY(expected > 0);
        QCOMPARE(list->count(), expected);
        qInfo() << "Extra recipes visible:" << list->count();
    }
    void selectedNamesAreExact() {
        MainWindow w;
        auto *list=w.findChild<QListWidget *>("listWidget");
        QListWidgetItem foo("[extra] foo - 1: First"), foobar("[extra] foobar - 2: Second");
        foo.setData(Qt::UserRole,"foo"); foobar.setData(Qt::UserRole,"foobar");
        test.clear();
        foo.setCheckState(Qt::Checked); list->itemClicked(&foo);
        foobar.setCheckState(Qt::Checked); list->itemClicked(&foobar);
        QCOMPARE(test,QString("foo foobar"));
        foo.setCheckState(Qt::Unchecked); list->itemClicked(&foo);
        QCOMPARE(test,QString("foobar"));
        QVERIFY(!test.contains("[extra]"));
        test.clear();
    }
};
QTEST_MAIN(VpacmanTest)
#include "vpacman_test.moc"
