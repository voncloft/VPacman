#include "mainwindow.h"
#include <QtTest>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTemporaryDir>
#include <QLabel>
class VpacmanTest : public QObject {
    Q_OBJECT
    static void writeFile(const QString &path, const QByteArray &contents) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE(file.write(contents), contents.size());
    }
    static void addRecipe(const QTemporaryDir &tmp, const QString &name, const QString &depends = {}) {
        QVERIFY(QDir().mkpath(tmp.filePath("repo/" + name)));
        // The resolver must read comments, never execute the recipe.
        writeFile(tmp.filePath("repo/" + name + "/spkgbuild"),
                  ("# description : Preview fixture\n# depends : " + depends
                   + "\nname=" + name + "\nversion=1\nexit 99\n").toUtf8());
    }
    static QListWidgetItem *packageItem(QListWidget *list, const QString &name) {
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->data(Qt::UserRole).toString() == name) return list->item(i);
        return nullptr;
    }
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
    void previewTracksSelectionsAndPendingDependencies() {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        QVERIFY(QFileInfo("/var/lib/scratchpkg/db/python-pip").size() > 0);
        addRecipe(tmp, "vp-test-foo", "vp-test-middle python-pip");
        addRecipe(tmp, "vp-test-foobar", "vp-test-shared");
        addRecipe(tmp, "vp-test-middle", "vp-test-shared");
        addRecipe(tmp, "vp-test-shared");
        addRecipe(tmp, "python-pip");
        const auto config = tmp.filePath("scratchpkg.repo");
        writeFile(config, (tmp.filePath("repo") + " https://example.invalid/repo\n").toUtf8());
        MainWindow window(nullptr, config);
        auto *list = window.findChild<QListWidget *>("listWidget");
        auto *label = window.findChild<QLabel *>("installPreviewLabel");
        auto *search = window.findChild<QPlainTextEdit *>("plainTextEdit");
        auto *process = window.findChild<QProcess *>("dependencyPreviewProcess");
        QCOMPARE(packageItem(list, "python-pip")->checkState(), Qt::Checked);
        QCOMPARE(label->text(), QString("Selected: none\nDependencies to install: none"));
        QCOMPARE(process->state(), QProcess::NotRunning);

        window.show();
        auto *foo = packageItem(list, "vp-test-foo");
        const QRect rect = list->visualItemRect(foo);
        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier,
                          QPoint(rect.left() + 8, rect.center().y()));
        QCOMPARE(foo->checkState(), Qt::Checked);
        packageItem(list, "vp-test-foobar")->setCheckState(Qt::Checked);
        QTRY_COMPARE(label->text(), QString("Selected: vp-test-foo, vp-test-foobar\nDependencies to install: vp-test-shared, vp-test-middle"));
        QCOMPARE(process->program(), QString("/usr/bin/scratch"));
        QCOMPARE(process->arguments(), QStringList({"deplist", "-q", "--nocolor", "--repo-file=" + config,
                                                    "vp-test-foo", "vp-test-foobar"}));
        const auto expected = label->text();
        search->setPlainText("__hidden__");
        QCOMPARE(list->count(), 0);
        QTRY_COMPARE(label->text(), expected);
        search->clear();
        packageItem(list, "vp-test-foo")->setCheckState(Qt::Unchecked);
        QTRY_COMPARE(label->text(), QString("Selected: vp-test-foobar\nDependencies to install: vp-test-shared"));
        packageItem(list, "vp-test-foobar")->setCheckState(Qt::Unchecked);
        QCOMPARE(label->text(), QString("Selected: none\nDependencies to install: none"));
        QTest::qWait(200);
        QCOMPARE(process->state(), QProcess::NotRunning);
        QVERIFY(window.findChild<QTextEdit *>("textEdit")->toPlainText().isEmpty());
    }
    void previewDiscardsStaleResultsAndReportsFailure() {
        QTemporaryDir tmp;
        QVERIFY(tmp.isValid());
        addRecipe(tmp, "vp-test-first");
        addRecipe(tmp, "vp-test-second");
        const auto config = tmp.filePath("scratchpkg.repo");
        writeFile(config, (tmp.filePath("repo") + "\n").toUtf8());
        const auto resolver = tmp.filePath("resolver");
        writeFile(resolver, "#!/bin/sh\nfor arg do target=$arg; done\nsleep 0.3\nprintf '%s\\n' \"dep-$target\" \"$target\"\n");
        QVERIFY(QFile::setPermissions(resolver, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        MainWindow window(nullptr, config, resolver);
        auto *list = window.findChild<QListWidget *>("listWidget");
        auto *label = window.findChild<QLabel *>("installPreviewLabel");
        auto *process = window.findChild<QProcess *>("dependencyPreviewProcess");
        packageItem(list, "vp-test-first")->setCheckState(Qt::Checked);
        QTRY_COMPARE(process->state(), QProcess::Running);
        packageItem(list, "vp-test-first")->setCheckState(Qt::Unchecked);
        packageItem(list, "vp-test-second")->setCheckState(Qt::Checked);
        QTRY_COMPARE(label->text(), QString("Selected: vp-test-second\nDependencies to install: dep-vp-test-second"));
        packageItem(list, "vp-test-second")->setCheckState(Qt::Unchecked);
        packageItem(list, "vp-test-first")->setCheckState(Qt::Checked);
        QTRY_COMPARE(process->state(), QProcess::Running);
        packageItem(list, "vp-test-first")->setCheckState(Qt::Unchecked);
        QTRY_COMPARE(process->state(), QProcess::NotRunning);
        QCOMPARE(label->text(), QString("Selected: none\nDependencies to install: none"));

        writeFile(resolver, "#!/bin/sh\necho 'resolver failed' >&2\nexit 1\n");
        packageItem(list, "vp-test-first")->setCheckState(Qt::Checked);
        QTRY_VERIFY(label->text().contains("unavailable (resolver failed)"));
        packageItem(list, "vp-test-first")->setCheckState(Qt::Unchecked);
        QVERIFY(QFile::remove(resolver));
        packageItem(list, "vp-test-first")->setCheckState(Qt::Checked);
        QTRY_VERIFY(label->text().contains("unavailable ("));
    }
    void liveAnkiPreview() {
        MainWindow window;
        window.findChild<QPlainTextEdit *>("plainTextEdit")->setPlainText("anki");
        auto *item = packageItem(window.findChild<QListWidget *>("listWidget"), "anki");
        QVERIFY(item);
        if (QFileInfo("/var/lib/scratchpkg/db/anki").size() > 0) QSKIP("Anki already installed");
        item->setCheckState(Qt::Checked);
        auto *label = window.findChild<QLabel *>("installPreviewLabel");
        QTRY_VERIFY_WITH_TIMEOUT(!label->text().contains("resolving"), 30000);
        QVERIFY(!label->text().contains("unavailable"));
        QVERIFY(label->text().startsWith("Selected: anki\nDependencies to install: "));
        window.show();
        QTest::qWait(100);
        auto *output = window.findChild<QTextEdit *>("textEdit");
        QVERIFY(label->mapTo(&window, QPoint()).y() >= output->mapTo(&window, QPoint()).y() + output->height());
        qInfo().noquote() << label->text();
        const QString screenshot = qEnvironmentVariable("VPACMAN_PREVIEW_SCREENSHOT");
        if (!screenshot.isEmpty()) QVERIFY(window.grab().save(screenshot));
    }
};
QTEST_MAIN(VpacmanTest)
#include "vpacman_test.moc"
