#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QMessageBox>
#include <QDirIterator>
#include <QString>
#include <QModelIndexList>
#include <QListWidgetItem>
#include <QProcess>


#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QAbstractItemView>
#include <QListWidget>
#include <QMessageBox>
#include <QDirIterator>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_pushButton_clicked();

    //void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void setStdout();
    void command(QString text);
    void fileRead(QString path);

    void on_actionAbout_triggered();

    void on_pushButton_2_clicked();
    void loadPackages();
    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *m_process;
    void resizeEvent(QResizeEvent*);
};
#endif // MAINWINDOW_H
