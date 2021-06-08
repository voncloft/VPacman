//todo: have checkversion loop thru files
//find way to convert the version strings to integers and do the <>=
//give upgrade options in the list

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
    //ui->plainTextEdit->setGeometry(2000,0,280,40);

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
        package=item->text().section(' ',0,1);

        test+=package+" ";
    }
    else
    {
        QString package;
        package=item->text().section(' ',0,1);

        test.remove(package);
    }
}

void MainWindow::on_pushButton_clicked()
{
    command("sudo /usr/bin/scratch install -yc " + test);
    //command ("echo test");
    QMessageBox msgbox;
    msgbox.setText("sudo /usr/bin/scratch install -yc " + test);
   // msgbox.exec();
}

void::MainWindow::setStdout()
{
    ui->textEdit->append( m_process->readAllStandardOutput() );
    //ui->plainTextEdit->appendPlainText(m_process->readAll());

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
//ui->plainTextEdit->setGeometry(MainWindow::width()-310,0,280,40);
//ui->textEdit->resize(MainWindow::width()-30,MainWindow::height());

//ui->textEdit->setGeometry(10,400,MainWindow::width(),MainWindow::height());
//ui->listWidget->resize(MainWindow::width()-30,300);

}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("VPacman");
    msgbox.setText("Voncloft Package Manager - Version 2.3 - A front end gui for scratchpkg");
    msgbox.exec();
}

void MainWindow::on_pushButton_2_clicked()
{
    loadPackages();
}
void MainWindow::loadPackages()
{
    QDirIterator dirIt("/usr/Voncloft-OS",QDirIterator::Subdirectories);
    int installed=0;

    QDirIterator dirIts("/var/lib/scratchpkg",QDirIterator::Subdirectories);
    QMessageBox msgbox;
    ui->listWidget->clear();

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
                    //msgbox.setText(dirIt.fileName());
                    //msgbox.exec();
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
                        QString names;
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


                        //dirIt.filePath();
                        QString category;

                        category=dirIt.filePath().remove("/usr/Voncloft-OS/");
                        int str;
                        str=category.indexOf('/');
                        int length=category.length();
                        QString final_category;
                        final_category=category.remove(str,length);
                        values->setText("[" + category + "] " + dirIt.fileName() + " - " + version.remove(0,8) + ": "+ description.remove(0,16));

                        if (QFileInfo("/var/lib/scratchpkg/db/"+dirIt.fileName()).exists())
                        {
                            values->setCheckState(Qt::Checked);
                            installed+=1;
                        }
                        else
                        {
                          values->setCheckState(Qt::Unchecked);
                        }

                        if(values->text().contains("- :"))
                        {
                        }
                        else
                        {

                            ui->listWidget->addItem(values);
                        }

                       //ui->listWidget->setItemWidget(values,new QCheckBox("zstd"));
                    }
                }
            }
        ui->listWidget->sortItems(Qt::AscendingOrder);
        QString installed_string="Packages installed: " + QString::number(installed);
        //installed_string
        ui->label->setText(installed_string);
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
//QString version1;
void MainWindow::searchPackages(QString term)
{
    QDirIterator dirIt("/usr/Voncloft-OS",QDirIterator::Subdirectories);

    QDirIterator dirIts("/var/lib/scratchpkg",QDirIterator::Subdirectories);
    QMessageBox msgbox;
    ui->listWidget->clear();
    int installed=0;
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
                                    file.close();


                                }

                                QString category;

                                category=dirIt.filePath().remove("/usr/Voncloft-OS/");
                                int str;
                                str=category.indexOf('/');
                                int length=category.length();
                                QString final_category;
                                final_category=category.remove(str,length);
                                values->setText("[" + category + "] " + dirIt.fileName() + " - " + version.remove(0,8) + ": "+ description.remove(0,16));


                                //values->setText(dirIt.fileName() + " - " + version.remove(0,8) + ": "+ description.remove(0,16));
                                if (QFileInfo("/var/lib/scratchpkg/db/"+dirIt.fileName()).exists())
                                {
                                    values->setCheckState(Qt::Checked);
                                    installed+=1;
                                }
                                else
                                {
                                  values->setCheckState(Qt::Unchecked);
                                }
                                if(values->text().contains(term))
                                {

                                    ui->listWidget->addItem(values);
                                }
                            }









                       //ui->listWidget->setItemWidget(values,new QCheckBox("zstd"));
                    }
                }
            }
        ui->listWidget->sortItems(Qt::AscendingOrder);
        QString installed_string="Packages installed: " + QString::number(installed);
        //installed_string
        //ui->label->setText(installed_string);
}
void versioncheck(QString filecheck1,QString filecheck2)
{
    QMessageBox msgbox;
    QFile file(filecheck2);
    QFile file2(filecheck1);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    if(!file2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }
    QTextStream in(&file);
    QTextStream in2(&file2);
    QString line2=in.readLine();
    QString final;
    QString version;
    QString release;
    while(!in2.atEnd()) {
        QString line = in2.readLine();
            if (line.contains("version="))
            {
                version=line.remove("version=");
            }
            else if (line.contains("release="))
            {
                release=line.remove("release=");
            }
        }
        final=version+" "+release;
        if (final==line2)
        {
               msgbox.setText("same");
               msgbox.exec();
        }
    file.close();
    file2.close();
}
void MainWindow::on_pushButton_5_clicked()
{
    versioncheck("/usr/Voncloft-OS/core/nano/spkgbuild","/var/lib/scratchpkg/db/nano");
}
