#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QString"
QString woot = "ping www.google.com";
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(setStdout()) );
    //connect(ui->woot, SIGNAL(returnPressed()), this, SLOT(command()) );
    loadPackages();

        }

MainWindow::~MainWindow()
{
    delete ui;
}

QString test;
void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    QMessageBox msgbox;
    if(item->checkState()==Qt::Checked)
    {
        QString package;
        package=item->text().section(' ',0,0);

        test+=package+" ";
    }
    else
    {
        QString package;
        package=item->text().section(' ',0,0);

        test.remove(package);
    }
}

void MainWindow::on_pushButton_clicked()
{
    command("sudo /usr/bin/scratch install -yc " + test);
    //command ("echo test");
    QMessageBox msgbox;
    msgbox.setText("sudo /usr/bin/scratch install -yc " + test);
    //msgbox.exec();
}

void::MainWindow::setStdout()
{
    ui->textEdit->append( m_process->readAllStandardOutput() );

}
void MainWindow::command(QString text)
{
    ui->textEdit->clear();
    QString strCommand;
    if(QSysInfo::productType()=="windows")
        strCommand = "cmd /C ";

    //strCommand = "y | sudo /usr/bin/scratch install -y " + test;
    strCommand=test;

    QMessageBox msgbox;
    msgbox.setText(text);
    //msgbox.exec();
    m_process->start(text);
    test.clear();

}
void MainWindow::resizeEvent(QResizeEvent*)
{


ui->plainTextEdit->setGeometry(MainWindow::width()-300,0,280,40);
ui->textEdit->resize(MainWindow::width()-30,MainWindow::height()-30);
ui->listWidget->resize(MainWindow::width()-30,300);
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("VPacman");
    msgbox.setText("Voncloft Package Manager - Version 2.2 - A front end gui for scratchpkg");
    msgbox.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    loadPackages();
}
void MainWindow::loadPackages()
{
    QDirIterator dirIt("/usr/Voncloft-OS",QDirIterator::Subdirectories);

    QDirIterator dirIts("/var/lib/scratchpkg",QDirIterator::Subdirectories);
    QMessageBox msgbox;
    ui->listWidget->clear();
    //msgbox.setText(dirIt.fileName());
    //msgbox.exec();
        while (dirIt.hasNext())
        {

            //while(dirIts.hasNext())
            //{
                dirIt.next();
                if(dirIt.fileName()==".." || dirIt.fileName()==".")
                {

                }
                else
                {
                    if (QFileInfo(dirIt.filePath()).isFile())
                    {
                    }
                    else
                    {

                        QListWidgetItem *values = new QListWidgetItem;
                        //fileRead(dirIt.filePath()+"/spkgbuild");

                        QString line;
                        QFile file(dirIt.filePath()+"/spkgbuild");
                        int i;
                        QString description;
                        QString version;

                        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
                                QTextStream stream(&file);
                                while (!stream.atEnd()){
                                    line = stream.readLine();
                                    if(line.contains("description"))
                                    {
                                        //ui->textEdit->append(line);
                                        description=line;

                                    }
                                    if(line.contains("version="))
                                    {
                                        version=line;

                                    }


                                }
                            }
                            file.close();



                        values->setText(dirIt.fileName() + " - " + version.remove(0,8) + ": "+ description.remove(0,16));
                        if (QFileInfo("/var/lib/scratchpkg/index/"+dirIt.fileName()+"/.pkginfo").exists())
                        {
                            values->setCheckState(Qt::Checked);
                        }
                        else
                        {
                          values->setCheckState(Qt::Unchecked);
                        }

                        ui->listWidget->addItem(values);
                       //ui->listWidget->setItemWidget(values,new QCheckBox("zstd"));
                    }
                }
            }
        ui->listWidget->sortItems(Qt::AscendingOrder);

}

void MainWindow::on_pushButton_3_clicked()
{
    command("sudo scratch sysup -yc");
}

void MainWindow::on_pushButton_4_clicked()
{
    command("sudo /usr/bin/outdated");
}
QString search_terms;
void MainWindow::on_plainTextEdit_textChanged()
{
    searchPackages(ui->plainTextEdit->toPlainText());
}
void MainWindow::searchPackages(QString term)
{
    QDirIterator dirIt("/usr/Voncloft-OS",QDirIterator::Subdirectories);

    QDirIterator dirIts("/var/lib/scratchpkg",QDirIterator::Subdirectories);
    QMessageBox msgbox;
    ui->listWidget->clear();
    //msgbox.setText(dirIt.fileName());
    //msgbox.exec();
        while (dirIt.hasNext())
        {

            //while(dirIts.hasNext())
            //{
                dirIt.next();
                if(dirIt.fileName()==".." || dirIt.fileName()==".")
                {

                }
                else
                {
                    if (QFileInfo(dirIt.filePath()).isFile())
                    {
                    }
                    else
                    {

                        QListWidgetItem *values = new QListWidgetItem;
                        //fileRead(dirIt.filePath()+"/spkgbuild");

                        QString line;
                        QFile file(dirIt.filePath()+"/spkgbuild");
                        int i;
                        QString description;
                        QString version;

                        if (file.open(QIODevice::ReadOnly | QIODevice::Text) && (file.fileName().contains(term))){
                                QTextStream stream(&file);
                                while (!stream.atEnd()){
                                    line = stream.readLine();
                                    if(line.contains("description"))
                                    {
                                        //ui->textEdit->append(line);
                                        description=line;

                                    }
                                    if(line.contains("version="))
                                    {
                                        version=line;

                                    }
                                    file.close();


                                }
                                values->setText(dirIt.fileName() + " - " + version.remove(0,8) + ": "+ description.remove(0,16));
                                if (QFileInfo("/var/lib/scratchpkg/index/"+dirIt.fileName()+"/.pkginfo").exists())
                                {
                                    values->setCheckState(Qt::Checked);
                                }
                                else
                                {
                                  values->setCheckState(Qt::Unchecked);
                                }

                                ui->listWidget->addItem(values);
                            }









                       //ui->listWidget->setItemWidget(values,new QCheckBox("zstd"));
                    }
                }
            }
        ui->listWidget->sortItems(Qt::AscendingOrder);

}
