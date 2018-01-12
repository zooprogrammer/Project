#include "register.h"
#include <QDebug>
Register::Register(QWidget *parent) : QDialog(parent)
{
    QPushButton * r;

    QBoxLayout * lay = new QVBoxLayout(this);
    lay->addWidget(username = new QLineEdit);
    lay->addWidget(password = new QLineEdit);
    lay->addWidget(password2 = new QLineEdit);
    lay->addWidget(r = new QPushButton("Sign up"));

    username->setPlaceholderText("Please input the name");
    password->setPlaceholderText("Please input the password");
    password2->setPlaceholderText("Please input the password again");

    password->setEchoMode(QLineEdit::Password);
    password2->setEchoMode(QLineEdit::Password);
    this->setWindowTitle("TO BE VIP");

    connect(r,SIGNAL(clicked()),this,SLOT(slotReg()));

    man = new QNetworkAccessManager(this);
    connect(man,SIGNAL(finished(QNetworkReply*)),
            this,SLOT(slotNetworkApply(QNetworkReply*)));
}

void Register::slotReg()
{
    QString username = this->username->text();
    QString password = this->password->text();
    QString password2 = this->password2->text();

    if(username.length() == 0|| password.length() == 0)
    {
        QMessageBox::warning(NULL,"Register failed","Input wrong");
        return;
    }

    if(password != password2)
    {
       QMessageBox::warning(NULL,"Register failed","Password wrong!");
       return;
    }

    QJsonObject obj;
    obj.insert("username",username);
    obj.insert("password",password);
    /*
    {
        username:xxx
        password:yyy
    }
    */
    //commit the info of registerion  libcurl??
    QNetworkRequest req;
    QString url = QString("http://%1:%2/reg").arg(SERVER_IP).arg(SERVER_PORT);
    qDebug() << url;

    QByteArray data = QJsonDocument(obj).toJson(); //json
    req.setUrl(QUrl(url));
}

void Register::slotNetworkApply(QNetworkReply *)
{

}
