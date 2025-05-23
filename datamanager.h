#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QString>

// Declaraciones adelantadas
class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject; // Para la construcción del cuerpo de la solicitud

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

    // Método para iniciar la llamada a la API de Gemini
    // Toma la ruta del archivo de imagen y el texto del prompt como entrada.
    void fetchDescriptionFromGemini(const QString &imagePath, const QString &promptText);

signals:
    // Señal emitida cuando la solicitud a la API comienza.
    // AppWidget puede conectarse a esta señal para, por ejemplo, deshabilitar botones.
    void processingStarted();

    // Señal emitida cuando la descripción de la imagen se ha generado exitosamente.
    // Contiene la descripción como un QString.
    void descriptionReady(const QString &description);

    // Señal emitida si ocurre un error durante el proceso de obtención de la descripción.
    // Contiene un mensaje de error.
    void errorOccurred(const QString &errorMessage);

    // Señal emitida cuando la solicitud a la API ha finalizado (ya sea con éxito o con error).
    // AppWidget puede conectarse a esta señal para rehabilitar botones.
    void processingFinished();

private slots:
    // Slot interno para manejar la respuesta de la API de Gemini cuando la solicitud de red finaliza.
    void handleGeminiReply();

private:
    QNetworkAccessManager *m_networkManager; // Administrador de acceso a la red para realizar las solicitudes HTTP.

    // Constantes para la API de Gemini
    // La API Key se almacena aquí de forma segura dentro de la clase.
    const QString GEMINI_API_KEY = "AIzaSyBUkiNojmeSk4FnPCtC4CRcINf4FOdAtA0";
    // Endpoint base de la API. La clave se añadirá a este endpoint.
    const QString GEMINI_API_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=";

    // Método privado para construir el cuerpo JSON de la solicitud a la API.
    QJsonObject prepareRequestPayload(const QString &imageBase64, const QString &mimeType, const QString &promptText);
};

#endif // DATAMANAGER_H
