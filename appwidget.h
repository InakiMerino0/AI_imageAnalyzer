#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QString>

// Declaraciones adelantadas
class DataManager; // <--- NUEVO: Declaración adelantada para DataManager
class DragDropArea;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

// class QNetworkAccessManager; // Ya no es necesario
// class QNetworkReply; // Ya no es necesario

class AppWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AppWidget(QWidget *parent = nullptr);
    ~AppWidget();

private slots:
    void handleImageDropped(const QString &filePath);
    void processInput();
    void displayDescription(const QString &description); // Este slot puede ser reutilizado
    void updateProcessButtonState();
    void onResetButtonClicked();

    // Nuevos slots para manejar señales de DataManager
    void handleProcessingStarted();
    void handleDescriptionReady(const QString &description);
    void handleErrorOccurred(const QString &errorMessage);
    void handleProcessingFinished();

signals:
    // Esta señal podría ya no ser necesaria si displayDescription se llama directamente,
    // pero la mantenemos por si se usa en otro lado o para futura flexibilidad.
    void descriptionGenerated(const QString &description);

private:
    DragDropArea *dragDropInput;
    QLabel *promptLabel;
    QLineEdit *promptLineEdit;
    QPushButton *processButton;
    QPushButton *resetButton;
    QLabel *responseLabel;
    QTextEdit *responseOutput;

    QString currentImagePath;
    // QNetworkAccessManager *networkManager; // <--- ELIMINADO
    DataManager *dataManager; // <--- NUEVO: Instancia de DataManager

    // void callGeminiApi(const QString &apiKey, const QString &imagePath, const QString &promptText); // <--- ELIMINADO
};

#endif // APPWIDGET_H
