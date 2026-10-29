//todo: have checkversion loop thru files
//find way to convert the version strings to integers and do the <>=
//give upgrade options in the list

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QString"
#include <QApplication>
#include <QIcon>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSignalBlocker>
QString woot = "ping www.google.com";
MainWindow::MainWindow(QWidget *parent, const QString &repoFile, const QString &scratchProgram)
    : QMainWindow(parent)
    , repositoryConfig(repoFile)
    , dependencyCommand(scratchProgram)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme("vpacman", QIcon(":/icons/vpacman.svg")));

    ui->plainTextEdit->setPlaceholderText("Search packages");
    ui->textEdit->setReadOnly(true);
    previewProcess = new QProcess(this);
    previewProcess->setObjectName("dependencyPreviewProcess");
    previewTimer = new QTimer(this);
    previewTimer->setSingleShot(true);
    previewTimer->setInterval(100);
    connect(previewTimer, &QTimer::timeout, this, &MainWindow::startInstallPreview);
    connect(previewProcess, &QProcess::finished, this, &MainWindow::finishInstallPreview);
    connect(previewProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart) return;
        if (runningPreviewRevision != previewRevision) {
            startInstallPreview();
            return;
        }
        ui->installPreviewLabel->setText(previewHeading + "\nDependencies to install: unavailable ("
                                         + previewProcess->errorString() + ")");
    });
    m_process = new QProcess(this);
    m_process->setObjectName("packageCommandProcess");
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        ui->textEdit->insertPlainText("\nProcess error: " + m_process->errorString() + "\n");
    });
    connect(m_process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) {
        setStdout();
        ui->textEdit->moveCursor(QTextCursor::End);
        if (status == QProcess::NormalExit && code == 0) {
            ui->textEdit->insertPlainText("\nDone.\n");
        } else if (status == QProcess::CrashExit) {
            ui->textEdit->insertPlainText("\nCommand failed (process crashed).\n");
        } else {
            ui->textEdit->insertPlainText(QString("\nCommand failed (exit %1).\n").arg(code));
        }
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        loadPackages();
    });
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(setStdout()) );
    //connect(ui->woot, SIGNAL(returnPressed()), this, SLOT(command()) );
    loadPackages();
    //ui->plainTextEdit->setGeometry(2000,0,280,40);

        }

MainWindow::~MainWindow()
{
    disconnect(previewProcess, nullptr, this, nullptr);
    if (previewProcess->state() != QProcess::NotRunning) {
        previewProcess->kill();
        previewProcess->waitForFinished(1000);
    }
    delete ui;
}

void MainWindow::on_listWidget_itemChanged(QListWidgetItem *item)
{
    const QString package = item->data(Qt::UserRole).toString();
    if (package.isEmpty()) return;
    if (item->checkState() == Qt::Checked) {
        if (!selectedPackages.contains(package)) selectedPackages.append(package);
    } else selectedPackages.removeAll(package);
    updateInstallPreview();
}

void MainWindow::updateInstallPreview()
{
    ++previewRevision;
    previewTimer->stop();
    previewTargets.clear();
    QStringList selected;
    for (const auto &package : selectedPackages) {
        const QFileInfo installed("/var/lib/scratchpkg/db/" + package);
        if (installed.isFile() && installed.size() > 0) {
            selected.append(package + " (already installed)");
        } else {
            selected.append(package);
            previewTargets.append(package);
        }
    }
    previewHeading = "Selected: " + (selected.isEmpty() ? QString("none") : selected.join(", "));
    ui->installPreviewLabel->setText(previewHeading + "\nDependencies to install: "
                                     + (previewTargets.isEmpty() ? "none" : "resolving…"));
    if (!previewTargets.isEmpty()) previewTimer->start();
}

void MainWindow::startInstallPreview()
{
    if (previewTargets.isEmpty() || previewProcess->state() != QProcess::NotRunning) return;
    runningPreviewRevision = previewRevision;
    // Use Scratch's own read-only resolver, including aliases and installed-package filtering.
    QStringList arguments = {"deplist", "-q", "--nocolor", "--repo-file=" + repositoryConfig};
    arguments.append(previewTargets);
    previewProcess->start(dependencyCommand, arguments);
}

void MainWindow::finishInstallPreview(int exitCode, QProcess::ExitStatus status)
{
    const QString output = QString::fromLocal8Bit(previewProcess->readAllStandardOutput());
    const QString errors = QString::fromLocal8Bit(previewProcess->readAllStandardError()).trimmed();
    // Rapid checkbox changes must never display a result for an older selection.
    if (runningPreviewRevision != previewRevision) {
        startInstallPreview();
        return;
    }
    if (status != QProcess::NormalExit || exitCode != 0) {
        ui->installPreviewLabel->setText(previewHeading + "\nDependencies to install: unavailable"
                                         + (errors.isEmpty() ? QString(" (Scratch failed)") : " (" + errors + ")"));
        return;
    }
    QStringList packages = output.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    packages.removeDuplicates();
    for (const auto &target : previewTargets) {
        if (!packages.contains(target)) {
            ui->installPreviewLabel->setText(previewHeading + "\nDependencies to install: unavailable (package not found: "
                                             + target + ")");
            return;
        }
    }
    for (const auto &package : selectedPackages) packages.removeAll(package);
    ui->installPreviewLabel->setText(previewHeading + "\nDependencies to install: "
                                     + (packages.isEmpty() ? QString("none") : packages.join(", ")));
}

void MainWindow::on_pushButton_clicked()
{
    command("sudo -n /usr/bin/scratch install -yc " + selectedPackages.join(' '));
    //command ("echo test");
    QMessageBox msgbox;
    msgbox.setText("sudo -n /usr/bin/scratch install -yc " + selectedPackages.join(' '));
   // msgbox.exec();
}

void MainWindow::setStdout()
{
    ui->textEdit->insertPlainText(QString::fromLocal8Bit(m_process->readAllStandardOutput()));
    ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    //ui->plainTextEdit->appendPlainText(m_process->readAll());

}
void MainWindow::command(QString text)
{
    if (m_process->state() != QProcess::NotRunning) {
        ui->textEdit->insertPlainText("\nA package command is already running.\n");
        return;
    }
    if ((text.contains("scratch install ") || text.contains("scratch upgrade ")) && selectedPackages.isEmpty()) {
        ui->textEdit->insertPlainText("Select a package first.\n");
        return;
    }
    ui->textEdit->clear();
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";

    //strCommand = "y | sudo -n /usr/bin/scratch install -y " + test;
    strCommand=selectedPackages.join(' ');

    QMessageBox msgbox;
    msgbox.setText(text);
    //msgbox.exec();
    m_process->startCommand(text);
    selectedPackages.clear();
    updateInstallPreview();

}
void MainWindow::resizeEvent(QResizeEvent*)
{
//ui->plainTextEdit->setGeometry(MainWindow::width()-310,0,280,40);
//ui->textEdit->resize(MainWindow::width()-30,MainWindow::height());

//ui->textEdit->setGeometry(10,400,MainWindow::width(),MainWindow::height());
//ui->listWidget->resize(MainWindow::width()-30,300);

}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("VPacman");
    msgbox.setText("Voncloft Package Manager " + QApplication::applicationVersion() + " — A graphical front end for Scratchpkg");
    msgbox.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    loadPackages();
}
void MainWindow::loadPackages()
{
    searchPackages(ui->plainTextEdit->toPlainText());
}

void MainWindow::on_pushButton_3_clicked()
{
    command("sudo -n /usr/bin/scratch sysup -yc");
}

void MainWindow::on_pushButton_4_clicked()
{
    command("/usr/bin/scratch outdate");
}
QString search_terms;
void MainWindow::on_plainTextEdit_textChanged()
{
    searchPackages(ui->plainTextEdit->toPlainText());
}
//QString version1;
void MainWindow::searchPackages(QString term)
{
    const QString query = term.trimmed();
    const QRegularExpression descriptionLine("^#\\s*description\\s*:\\s*(.*)$");
    const QSignalBlocker blocker(ui->listWidget);
    ui->listWidget->clear();

    QFile config(repositoryConfig);
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->label->setText("Cannot read repositories: " + repositoryConfig);
        updateInstallPreview();
        return;
    }
    QStringList repositories;
    QTextStream configStream(&config);
    while (!configStream.atEnd()) {
        const QString line = configStream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        // Scratch uses the first whitespace-separated field as the local repo path.
        const QString repo = QDir::cleanPath(line.split(QRegularExpression("\\s+")).first());
        if (!repositories.contains(repo)) repositories.append(repo);
    }

    QStringList unavailable;
    int installed = 0;
    for (const QString &repo : repositories) {
        const QDir categoryDir(repo);
        if (!categoryDir.exists() || !QFileInfo(repo).isReadable()) {
            unavailable.append(repo);
            continue;
        }
        const QString category = categoryDir.dirName();
        for (const QString &name : categoryDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QFile recipe(categoryDir.filePath(name + "/spkgbuild"));
            if (!recipe.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
            QString description, version;
            QTextStream stream(&recipe);
            while (!stream.atEnd()) {
                const QString line = stream.readLine();
                const auto match = descriptionLine.match(line);
                if (match.hasMatch()) description = match.captured(1);
                if (line.startsWith("version=")) version = line.mid(8).trimmed();
            }
            const QString display = "[" + category + "] " + name + " - " + version + ": " + description;
            if (!display.contains(query, Qt::CaseInsensitive)) continue;
            auto *item = new QListWidgetItem(display, ui->listWidget);
            item->setData(Qt::UserRole, name);
            const bool isInstalled = QFileInfo::exists("/var/lib/scratchpkg/db/" + name);
            item->setCheckState(isInstalled || selectedPackages.contains(name) ? Qt::Checked : Qt::Unchecked);
            if (isInstalled) ++installed;
        }
    }
    ui->listWidget->sortItems(Qt::AscendingOrder);
    QString summary = QString("Packages: %1 | Installed: %2 | Repositories: %3")
                          .arg(ui->listWidget->count()).arg(installed).arg(repositories.size());
    if (!unavailable.isEmpty()) summary += " | Unavailable: " + unavailable.join(", ");
    ui->label->setText(summary);
    ui->label->setWordWrap(true);
    updateInstallPreview();
}

void MainWindow::on_pushButton_5_clicked()
{
    command("sudo -n /usr/bin/scratch upgrade -yc " + selectedPackages.join(' '));
}
