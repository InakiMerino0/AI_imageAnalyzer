#include "ventana.h"
#include "ui_ventana.h"

ventana::ventana(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ventana)
{
    ui->setupUi(this);
}

ventana::~ventana()
{
    delete ui;
}
