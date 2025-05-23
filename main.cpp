#include "appwidget.h" // Nuestro widget principal
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Objeto de aplicación Qt

    AppWidget w; // Creamos una instancia de nuestro AppWidget
    w.show();    // Mostramos la ventana

    return a.exec(); // Iniciamos el bucle de eventos de la aplicación
}
