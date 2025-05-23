#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui {
class Login;
}
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_pbIngresar_clicked();
    void onNetworkReply(QNetworkReply *reply);
    void on_pbRegistrarse_clicked();

private:
    Ui::Login *ui;
    QNetworkAccessManager *networkManager;
};

#endif // LOGIN_H
