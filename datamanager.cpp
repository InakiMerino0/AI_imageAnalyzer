#include "datamanager.h"

#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DataManager::DataManager(QObject *parent) : QObject(parent)
{
    // Inicializa el QNetworkAccessManager.
    // Este objeto se usará para todas las solicitudes de red que DataManager realice.
    m_networkManager = new QNetworkAccessManager(this);
}

DataManager::~DataManager()
{
    // El QNetworkAccessManager será eliminado automáticamente debido a la jerarquía de padres de QObject,
    // pero es buena práctica asegurarse de que se limpie si es necesario, aunque aquí no lo es explícitamente.
}

// Método privado para construir el payload JSON para la API de Gemini.
QJsonObject DataManager::prepareRequestPayload(const QString &imageBase64, const QString &mimeType, const QString &promptText)
{
    QJsonObject jsonDataPart;
    jsonDataPart["mime_type"] = mimeType;
    jsonDataPart["data"] = imageBase64;

    QJsonObject inlineDataPart;
    inlineDataPart["inline_data"] = jsonDataPart;

    QJsonObject textPart;
    textPart["text"] = promptText;

    QJsonArray partsArray;
    partsArray.append(textPart);
    partsArray.append(inlineDataPart);

    QJsonObject contentEntry;
    contentEntry["parts"] = partsArray;

    QJsonArray contentsArray;
    contentsArray.append(contentEntry);

    QJsonObject generationConfig;
    generationConfig["temperature"] = 0.4;
    generationConfig["topK"] = 32;
    generationConfig["topP"] = 1.0;
    generationConfig["maxOutputTokens"] = 2048; // Ajusta según sea necesario

    QJsonObject requestBodyJson;
    requestBodyJson["contents"] = contentsArray;
    requestBodyJson["generationConfig"] = generationConfig;

    return requestBodyJson;
}

void DataManager::fetchDescriptionFromGemini(const QString &imagePath, const QString &promptText)
{
    // Emitir señal de que el procesamiento ha comenzado.
    emit processingStarted();

    QFile imageFile(imagePath);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Error: No se pudo abrir el archivo de imagen: %1").arg(imageFile.errorString()));
        emit processingFinished();
        return;
    }
    QByteArray imageData = imageFile.readAll();
    QString imageBase64 = QString::fromLatin1(imageData.toBase64());
    imageFile.close();

    QMimeDatabase db;
    QString mimeType = db.mimeTypeForFile(imagePath).name();
    if (mimeType.isEmpty() || !mimeType.startsWith("image/")) {
        QFileInfo fileInfo(imagePath);
        QString extension = fileInfo.suffix().toLower();
        if (extension == "jpg" || extension == "jpeg") mimeType = "image/jpeg";
        else if (extension == "png") mimeType = "image/png";
        // Añade más tipos MIME si es necesario (ej: "image/webp", "image/heic", "image/gif")
        // Gemini Vision actualmente soporta: PNG, JPEG, WEBP, HEIC, HEIF
        else {
            emit errorOccurred(QString("Error: Tipo de imagen no soportado o no reconocido: %1. Tipos comunes: PNG, JPEG.").arg(extension));
            emit processingFinished();
            return;
        }
    }

    // Construir el cuerpo de la solicitud JSON utilizando el método auxiliar.
    QJsonObject requestBodyJson = prepareRequestPayload(imageBase64, mimeType, promptText);
    QJsonDocument jsonDoc(requestBodyJson);
    QByteArray postData = jsonDoc.toJson();

    // Construir la URL completa para la API.
    QUrl apiUrl(GEMINI_API_ENDPOINT + GEMINI_API_KEY);
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Realizar la solicitud POST a la API.
    QNetworkReply *reply = m_networkManager->post(request, postData);

    // Conectar la señal 'finished' de QNetworkReply al slot 'handleGeminiReply' de esta clase.
    // Esto asegura que cuando la respuesta de la red esté completa, nuestro slot será llamado.
    connect(reply, SIGNAL(finished()), this, SLOT(handleGeminiReply()));
}

void DataManager::handleGeminiReply()
{
    // Obtener el objeto QNetworkReply que emitió la señal.
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        emit errorOccurred("Error: No se pudo obtener el objeto QNetworkReply del emisor de la señal.");
        emit processingFinished();
        return;
    }

    // Procesar la respuesta de la API.
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject.contains("candidates")) {
            QJsonArray candidates = jsonObject["candidates"].toArray();
            if (!candidates.isEmpty()) {
                QJsonObject firstCandidate = candidates[0].toObject();
                QJsonObject content = firstCandidate["content"].toObject();
                QJsonArray parts = content["parts"].toArray();
                if (!parts.isEmpty()) {
                    QString generatedText = parts[0].toObject()["text"].toString();
                    emit descriptionReady(generatedText); // Emitir descripción obtenida.
                } else {
                    emit errorOccurred("Respuesta de Gemini recibida, pero sin partes de texto.");
                }
            } else {
                emit errorOccurred("Respuesta de Gemini recibida, pero sin candidatos.");
            }
        } else if (jsonObject.contains("error")) {
            QJsonObject errorObj = jsonObject["error"].toObject();
            QString errorMessage = errorObj["message"].toString();
            int errorCode = errorObj["code"].toInt(); // Código de error HTTP
            QString errorStatus = errorObj["status"].toString(); // Estado del error (ej. "INVALID_ARGUMENT")

            // Aquí puedes personalizar el mensaje de error basado en errorCode o errorStatus
            // Por ejemplo, si errorCode es 400 y errorStatus es "INVALID_ARGUMENT",
            // podría ser que la API Key sea inválida o malformada.
            if (errorCode == 400 && errorStatus == "INVALID_ARGUMENT" && errorMessage.contains("API key not valid")) {
                emit errorOccurred(QString("Error de la API de Gemini: La clave API no es válida. Verifica que esté configurada correctamente en DataManager. (Detalle: %1)").arg(errorMessage));
            } else if (errorMessage.contains(" Billing account not found") || errorMessage.contains("billing account") || errorCode == 403) {
                emit errorOccurred(QString("Error de la API de Gemini: Problema con la cuenta de facturación o permisos. (Detalle: %1)").arg(errorMessage));
            }
            else {
                emit errorOccurred(QString("Error de la API de Gemini: %1 (Código: %2, Estado: %3)").arg(errorMessage).arg(errorCode).arg(errorStatus));
            }

        } else if (responseData.isEmpty() && jsonResponse.isNull()) {
            // A veces, si la URL está mal o hay un problema de red no capturado antes, la respuesta puede ser vacía.
            emit errorOccurred("Respuesta de Gemini vacía o no es un JSON válido. Verifica la URL de la API y la conexión.");
        }
        else {
            // Para depuración, si la estructura del JSON no es la esperada.
            // QString rawResponse = QString::fromUtf8(responseData); // Cuidado con datos muy grandes
            // emit errorOccurred(QString("Respuesta JSON de Gemini no reconocida o vacía. Respuesta cruda: %1").arg(rawResponse.left(500)));
            emit errorOccurred("Respuesta JSON de Gemini no reconocida o con formato inesperado.");
        }
    } else {
        // Error de red (ej: no se pudo conectar al servidor, timeout).
        QString networkError = reply->errorString();
        QString serverResponse = QString::fromUtf8(reply->readAll()); // Contenido de la respuesta del servidor si hubo una.
        emit errorOccurred(QString("Error de red: %1\nRespuesta del servidor (si la hay):\n%2").arg(networkError).arg(serverResponse));
    }

    // Asegurarse de que la señal processingFinished se emita después de procesar la respuesta.
    emit processingFinished();
    // Eliminar el objeto QNetworkReply para liberar recursos.
    reply->deleteLater();
}
