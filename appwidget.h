#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QString>

// Declaraciones adelantadas para el módulo de red
class QNetworkAccessManager;
class QNetworkReply; // Ya no se pasa como argumento al slot onGeminiApiReplyFinished

// Otras declaraciones adelantadas
class DragDropArea;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class AppWidget : public QWidget
{
    Q_OBJECT // Necesario para la sintaxis SIGNAL/SLOT

public:
    explicit AppWidget(QWidget *parent = nullptr);
    ~AppWidget();

private slots: // Los slots deben estar en una sección 'slots' (o public/private slots)
    void handleImageDropped(const QString &filePath);
    void processInput();
    void displayDescription(const QString &description);
    void updateProcessButtonState();
    void onGeminiApiReplyFinished();
    void onResetButtonClicked();

signals: // Las señales deben estar en una sección 'signals'
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
    QNetworkAccessManager *networkManager;

    void callGeminiApi(const QString &apiKey, const QString &imagePath, const QString &promptText);
};

#endif // APPWIDGET_H
