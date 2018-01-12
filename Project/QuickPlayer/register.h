#ifndef REGISTER_H
#define REGISTER_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "def.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

class Register : public QDialog
{
    Q_OBJECT
public:

    explicit Register(QWidget *parent = nullptr);

    QLineEdit * username;
    QLineEdit * password;
    QLineEdit * password2;

    QNetworkAccessManager* man;

signals:

public slots:
    void slotReg();
    void slotNetworkApply(QNetworkReply*);
};

#endif // REGISTER_H
