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
QString woot = "ping www.google.com";
MainWindow::MainWindow(QWidget *parent, const QString &repoFile)
    : QMainWindow(parent)
    , repositoryConfig(repoFile)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme("vpacman", QIcon(":/icons/vpacman.svg")));

    ui->plainTextEdit->setPlaceholderText("Search packages");
    ui->textEdit->setReadOnly(true);
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        ui->textEdit->insertPlainText("\nProcess error: " + m_process->errorString() + "\n");
    });
    connect(m_process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus) {
        setStdout();
        ui->textEdit->insertPlainText(QString("\nCommand finished (exit %1).\n").arg(code));
        loadPackages();
    });
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(setStdout()) );
    //connect(ui->woot, SIGNAL(returnPressed()), this, SLOT(command()) );
    loadPackages();
    //ui->plainTextEdit->setGeometry(2000,0,280,40);

        }

MainWindow::~MainWindow()
{
    delete ui;
}

QString test;
void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    const QString package = item->data(Qt::UserRole).toString();
    if (package.isEmpty()) return;
    QStringList selected = test.split(' ', Qt::SkipEmptyParts);
    if (item->checkState() == Qt::Checked) {
        if (!selected.contains(package)) selected.append(package);
    } else selected.removeAll(package);
    test = selected.join(' ');
}

void MainWindow::on_pushButton_clicked()
{
    command("sudo -n /usr/bin/scratch install -yc " + test);
    //command ("echo test");
    QMessageBox msgbox;
    msgbox.setText("sudo -n /usr/bin/scratch install -yc " + test);
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
    if ((text.contains("scratch install ") || text.contains("scratch upgrade ")) && test.isEmpty()) {
        ui->textEdit->insertPlainText("Select a package first.\n");
        return;
    }
    ui->textEdit->clear();
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";

    //strCommand = "y | sudo -n /usr/bin/scratch install -y " + test;
    strCommand=test;

    QMessageBox msgbox;
    msgbox.setText(text);
    //msgbox.exec();
    m_process->startCommand(text);
    test.clear();

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
    const QStringList selected = test.split(' ', Qt::SkipEmptyParts);
    ui->listWidget->clear();

    QFile config(repositoryConfig);
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->label->setText("Cannot read repositories: " + repositoryConfig);
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
            item->setCheckState(isInstalled || selected.contains(name) ? Qt::Checked : Qt::Unchecked);
            if (isInstalled) ++installed;
        }
    }
    ui->listWidget->sortItems(Qt::AscendingOrder);
    QString summary = QString("Packages: %1 | Installed: %2 | Repositories: %3")
                          .arg(ui->listWidget->count()).arg(installed).arg(repositories.size());
    if (!unavailable.isEmpty()) summary += " | Unavailable: " + unavailable.join(", ");
    ui->label->setText(summary);
    ui->label->setWordWrap(true);
}

void MainWindow::on_pushButton_5_clicked()
{
    command("sudo -n /usr/bin/scratch upgrade -yc " + test);
}
