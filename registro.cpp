#include "registro.h"
#include "ui_registro.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

Registro::Registro(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Registro)
{
    ui->setupUi(this);
}

Registro::~Registro()
{
    delete ui;
}

void Registro::on_pbRegistrar_clicked()
{
    QJsonObject json;
    json["usuario"] = ui->leUsuario->text();
    json["clave"] = ui->leClave->text();
    json["apellido"] = ui->leApellido->text();
    json["mail"] = ui->leMail->text();

    QNetworkRequest request(QUrl("http://18.231.0.99:8000/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(this, "Registro", "¡Usuario registrado con éxito!");
            emit registroExitoso(ui->leUsuario->text());
            close();
        } else {
            QMessageBox::critical(this, "Error", reply->readAll());
        }
        reply->deleteLater();
    });
}
