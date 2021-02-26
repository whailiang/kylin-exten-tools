#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    process = new QProcess;
    ui->textEdit->hide();
    ui->label_5->hide();
    this->setWindowTitle(tr("麒麟软件包扩展属性工具"));
    ui->lineEdit->setReadOnly(true);
    this->setFixedSize(800,600);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    process->start("rm /tmp/kre -r");
    process->waitForFinished(-1);
    process->start("mkdir /tmp/kre");
    process->waitForFinished(-1);
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择软件包"),
                                                    "/home",
                                                    tr("deb (*.deb)"));
    QFileInfo file(fileName);
    filename = file.fileName();
    ui->lineEdit->setText(fileName);
    QString shell = "dpkg-deb -e " + fileName + " /tmp/kre/DEBIAN/";
    ui->label_5->setText("解包中...");
    ui->label_5->show();
    qDebug() << shell;
    process->start(shell);
    process->waitForFinished(-1);

    QString shell2 = "dpkg-deb -x " + fileName + " /tmp/kre/";
    process->start(shell2);
    process->waitForFinished(-1);

    QFile readfile("/tmp/kre/DEBIAN/control");
    if (!readfile.open(QFileDevice::ReadOnly)) {
        qDebug() << "contorl file open failed!";
        exit(0);
    }
    ui->textEdit->setText(QString(readfile.readAll()));
    ui->textEdit->show();
    ui->label_5->setText("软件属性");
    readfile.close();
}

void Widget::on_pushButton_2_clicked()
{
    if (ui->lineEdit_2->text().isEmpty() || ui->lineEdit_3->text().isEmpty()) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("麒麟软件包扩展属性工具"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QObject::tr("扩展属性不得为空"));
        QPushButton *btn = new QPushButton();
        btn->setText(QObject::tr("确定"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        return ;
    }
    QFile readfile("/tmp/kre/DEBIAN/control");
    if (!readfile.open(QIODevice::ReadOnly)) {
        qDebug() << "contorl read file open failed!";
    }
    QString text;
    QTextStream in(&readfile);
    QString str = in.readLine();
    while (!str.isNull()) {
        if (str.compare(""))
            text = text + str + "\n";
        str = in.readLine();
    }
    readfile.close();
    if (text.contains("XSBC-KyRuntimeEnv") && !ui->lineEdit_2->text().compare("XSBC-KyRuntimeEnv")) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QObject::tr("麒麟软件包扩展属性工具"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QObject::tr("扩展属性已经存在"));
        QPushButton *btn = new QPushButton();
        btn->setText(QObject::tr("确定"));
        msgBox.addButton(btn, QMessageBox::AcceptRole);
        msgBox.exec();
        return ;
    }

    QFile writeFile("/tmp/kre/DEBIAN/control");
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "contorl write file open failed!";
    }
    QTextStream out(&writeFile);
    out << text;
    out << ui->lineEdit_2->text()  << ": " << ui->lineEdit_3->text() << "\n";
    writeFile.flush();
    writeFile.close();


    if (!readfile.open(QFileDevice::ReadOnly)) {
        qDebug() << "contorl read file open failed!";
        exit(0);
    }
    ui->textEdit->setText(QString(readfile.readAll()));
    ui->textEdit->show();
}

void Widget::on_pushButton_3_clicked()
{
    ui->label_4->setText(tr("正在重新打包"));
    QString shell  = "mkdir /tmp/kre-deb";
    process->start(shell);
    process->waitForFinished(-1);


    QString username = QString::fromLatin1(getenv("SUDO_USER"));

    shell.clear();
    shell = "dpkg-deb -b /tmp/kre/ /home/" + username + "/" + filename;
    if (system(shell.toStdString().c_str())  < 0) {
        ui->label_4->setText(tr("重新打包失败!"));
    } else {
        ui->label_4->setText(tr("重新打包成功，重新打包后的文件在/home/%1中").arg(username));
    }
}
