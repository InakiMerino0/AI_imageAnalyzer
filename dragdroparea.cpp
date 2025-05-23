#include "dragdroparea.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QPaintEvent>  // <-- Incluir
#include <QPainter>     // <-- Incluir para dibujar
#include <QVBoxLayout>
#include <QMimeDatabase>
#include <QFileInfo>

DragDropArea::DragDropArea(QWidget *parent) : QWidget(parent)
{
    setAcceptDrops(true);

    infoLabel = new QLabel("Arrastra y suelta una imagen aquí", this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);
    // El infoLabel estará encima, así que el paintEvent dibujará la imagen debajo.
    // Podríamos ocultar infoLabel cuando hay imagen, o hacerlo semitransparente.
    // Por simplicidad inicial, lo dejaremos visible y el paintEvent dibujará la imagen.
    // Si la imagen llena el área, el label no se verá a menos que ajustemos su posición/tamaño.
    // Una mejor aproximación es ocultar el label cuando la imagen se muestra.

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel); // infoLabel sigue en el layout
    setLayout(layout);

    setStyleSheet("DragDropArea { border: 2px dashed #aaa; background-color: #f0f0f0; }");
    setMinimumSize(200, 150); // O el tamaño que prefieras para la vista previa
}

void DragDropArea::clearDroppedImage()
{
    currentFilePathDropped.clear();
    previewImage = QImage(); // Asigna una QImage nula para limpiarla
    infoLabel->setText("Arrastra y suelta una imagen aquí");
    infoLabel->setVisible(true); // Asegurarse de que el label sea visible
    setStyleSheet("DragDropArea { border: 2px dashed #aaa; background-color: #f0f0f0; }");
    update(); // Pide un redibujado para que se borre la imagen anterior
}

void DragDropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            QMimeDatabase db;
            QString mimeType = db.mimeTypeForUrl(urls.first()).name();
            if (mimeType.startsWith("image/")) {
                event->acceptProposedAction();
                if (previewImage.isNull()) { // Solo cambiar texto si no hay preview activa
                    infoLabel->setText("Suelta la imagen...");
                }
                setStyleSheet("DragDropArea { border: 2px solid green; background-color: #e0ffe0; }");
                return;
            }
        }
    }
    event->ignore();
}

void DragDropArea::dragMoveEvent(QDragMoveEvent *event)
{
    // Similar a dragEnterEvent, acepta si es un tipo de imagen válido
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().first().isLocalFile()) {
        QMimeDatabase db;
        if (db.mimeTypeForUrl(event->mimeData()->urls().first()).name().startsWith("image/")) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void DragDropArea::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (!filePath.isEmpty()) {
                QMimeDatabase db;
                QString mimeType = db.mimeTypeForFile(filePath).name();
                if (mimeType.startsWith("image/")) {
                    QImage tempImage(filePath); // Intentar cargar la imagen
                    if (!tempImage.isNull()) {
                        previewImage = tempImage;
                        currentFilePathDropped = filePath; // Guardar la ruta del archivo cargado
                        infoLabel->setVisible(false); // Ocultar el label para mostrar la imagen
                        setStyleSheet("DragDropArea { border: 1px solid #777; background-color: #e0e0e0; }"); // Estilo con imagen
                        update(); // Solicitar redibujado para mostrar la imagen
                        emit imageDropped(currentFilePathDropped);
                        event->acceptProposedAction();
                    } else {
                        // Falló la carga de la imagen con QImage
                        previewImage = QImage(); // Asegurar que esté nula
                        currentFilePathDropped.clear();
                        infoLabel->setText("Error: No se pudo cargar la vista previa de la imagen.");
                        infoLabel->setVisible(true);
                        setStyleSheet("DragDropArea { border: 2px dashed #cc0000; background-color: #f0f0f0; }"); // Borde rojo para error
                        update();
                        event->ignore();
                    }
                    return;
                }
            }
        }
    }
    // Si no es una URL válida o no es una imagen
    previewImage = QImage();
    currentFilePathDropped.clear();
    infoLabel->setText("Error: El archivo no es una imagen válida.");
    infoLabel->setVisible(true);
    setStyleSheet("DragDropArea { border: 2px dashed #cc0000; background-color: #f0f0f0; }");
    update();
    event->ignore();
}

void DragDropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    if (previewImage.isNull()) { // Si no hay vista previa activa
        infoLabel->setText("Arrastra y suelta una imagen aquí");
        setStyleSheet("DragDropArea { border: 2px dashed #aaa; background-color: #f0f0f0; }");
    } else { // Si hay una imagen cargada, mantener su estilo y el label oculto
        setStyleSheet("DragDropArea { border: 1px solid #777; background-color: #e0e0e0; }");
        // infoLabel->setVisible(false); // Ya debería estarlo
    }
}

void DragDropArea::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event); // Importante para dibujar el fondo y borde del widget según stylesheet

    if (!previewImage.isNull()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true); // También aplica a QImage escalada

        QSize widgetSize = this->size();
        // Escalar la imagen para que quepa en el widget manteniendo la relación de aspecto
        QImage scaledImage = previewImage.scaled(widgetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Calcular la posición para centrar la imagen escalada
        int x = (widgetSize.width() - scaledImage.width()) / 2;
        int y = (widgetSize.height() - scaledImage.height()) / 2;

        painter.drawImage(x, y, scaledImage);
    }
    // Si previewImage es nula, no se dibuja nada aquí, y el infoLabel (si está visible)
    // mostrará el texto correspondiente.
}
