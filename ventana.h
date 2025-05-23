#ifndef VENTANA_H
#define VENTANA_H

#include <QWidget>

namespace Ui {
class ventana;
}

class ventana : public QWidget
{
    Q_OBJECT

public:
    explicit ventana(QWidget *parent = nullptr);
    ~ventana();

private:
    Ui::ventana *ui;
};

#endif // VENTANA_H
