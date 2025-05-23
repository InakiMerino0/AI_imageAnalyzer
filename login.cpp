#include "registro.h"
#include "login.h"
#include "appwidget.h"
#include "ui_login.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QUrl>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &Login::on_pbIngresar_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &Login::on_pbRegistrarse_clicked);

}

Login::~Login()
{
    delete ui;
}

void Login::on_pbIngresar_clicked()
{
    QString usuario = ui->leUsuario->text();
    QString clave = ui->leClave->text();

    QJsonObject json;
    json["usuario"] = usuario;
    json["clave"] = clave;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QUrl url("http://18.231.0.99:8000/login");  // Reemplazá con la IP de tu instancia
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        onNetworkReply(reply);
    });
}

void Login::onNetworkReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();

        if (obj.contains("access_token")) {
            QString token = obj["access_token"].toString();
            QMessageBox::information(this, "Éxito", "Login exitoso. Token:\n" + token);
            this->close();
            AppWidget* ventana = new AppWidget();
            ventana->show();
        } else {
            QMessageBox::warning(this, "Error", "Respuesta inválida del servidor.");
        }
    } else {
        QMessageBox::critical(this, "Fallo", "Error de red: " + reply->errorString());
    }

    reply->deleteLater();
}

void Login::on_pbRegistrarse_clicked()
{
    Registro *registro = new Registro(this);
    connect(registro, &Registro::registroExitoso, this, [=](QString usuario){
        ui->leUsuario->setText(usuario);
    });
    registro->show();
}
