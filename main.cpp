#include "appwidget.h" // Nuestro widget principal
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Objeto de aplicación Qt

    QFile styleFile(":/styles.qss");  // Importante: el ":" indica recurso embebido
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
    }

    AppWidget w; // Creamos una instancia de nuestro AppWidget
    w.show();    // Mostramos la ventana

    return a.exec(); // Iniciamos el bucle de eventos de la aplicación
}
