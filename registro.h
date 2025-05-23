#ifndef REGISTRO_H
#define REGISTRO_H

#include <QWidget>

namespace Ui {
class Registro;
}

class Registro : public QWidget
{
    Q_OBJECT

public:
    explicit Registro(QWidget *parent = nullptr);
    ~Registro();

signals:
    void registroExitoso(QString usuario);

private slots:
    void on_pbRegistrar_clicked();

private:
    Ui::Registro *ui;
};

#endif // REGISTRO_H
