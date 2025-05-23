#include "appwidget.h"
#include "dragdroparea.h" // Asegúrate que DragDropArea esté incluido

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout> // Para el layout de botones
#include <QLabel>
#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QMimeDatabase>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const QString GEMINI_API_ENDPOINT_TEMPLATE = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=AIzaSyCMKmlbUYJb-ln5Cy2ytGGbGDPCoVe_FpE";
const QString MI_API_KEY_PRIVADA = "AIzaSyCMKmlbUYJb-ln5Cy2ytGGbGDPCoVe_FpE"; // RECUERDA PONER TU CLAVE AQUÍ

AppWidget::AppWidget(QWidget *parent)
    : QWidget(parent)
{
    networkManager = new QNetworkAccessManager(this);
    dragDropInput = new DragDropArea(this);

    promptLabel = new QLabel("¿Qué información deseas obtener de la imagen?:", this);
    promptLineEdit = new QLineEdit(this);
    promptLineEdit->setPlaceholderText("Ej: ¿Qué objetos hay? Describe los colores.");

    processButton = new QPushButton("Generar Descripción", this); // Texto original del botón
    processButton->setEnabled(false);

    resetButton = new QPushButton("Reiniciar Campos", this); // Crear el botón de reinicio

    responseLabel = new QLabel("Respuesta de Gemini:", this);
    responseOutput = new QTextEdit(this);
    responseOutput->setReadOnly(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel("Paso 1: Arrastra una imagen al recuadro", this));
    mainLayout->addWidget(dragDropInput);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(promptLabel);
    mainLayout->addWidget(promptLineEdit);
    mainLayout->addSpacing(10);

    // Layout horizontal para los botones "Generar" y "Reiniciar"
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(processButton);
    buttonLayout->addStretch(); // Opcional, para empujar el botón de reinicio a la derecha si se añaden más
    buttonLayout->addWidget(resetButton);
    mainLayout->addLayout(buttonLayout); // Añadir este layout al principal
    mainLayout->addSpacing(10);

    mainLayout->addWidget(responseLabel);
    mainLayout->addWidget(responseOutput);

    setLayout(mainLayout);

    // Conexiones usando SIGNAL y SLOT
    connect(dragDropInput, SIGNAL(imageDropped(const QString&)), this, SLOT(handleImageDropped(const QString&)));
    connect(processButton, SIGNAL(clicked()), this, SLOT(processInput()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetButtonClicked())); // <-- CONEXIÓN NUEVA
    connect(this, SIGNAL(descriptionGenerated(const QString&)), this, SLOT(displayDescription(const QString&)));
    connect(promptLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProcessButtonState()));

    setWindowTitle("Visor de Imágenes con Gemini API (Uso Privado)");
    resize(550, 650);
}

AppWidget::~AppWidget()
{
    // Destructor
}

void AppWidget::handleImageDropped(const QString &filePath)
{
    currentImagePath = filePath;
    responseOutput->clear(); // Limpiar respuesta anterior al cargar nueva imagen
    QFileInfo fileInfo(filePath);
    responseOutput->append(QString("Imagen '%1' lista para procesar.").arg(fileInfo.fileName()));
    updateProcessButtonState();
}

void AppWidget::processInput()
{
    const QString apiKey = MI_API_KEY_PRIVADA;
    if (apiKey == "TU_PROPIA_API_KEY_AQUI" || apiKey.isEmpty()) {
        responseOutput->setText("Error: La clave API no ha sido configurada en el código.\n"
                                "Por favor, edita appwidget.cpp y reemplaza "
                                "'TU_PROPIA_API_KEY_AQUI' con tu clave real.");
        return;
    }
    QString prompt = promptLineEdit->text().trimmed();
    if (currentImagePath.isEmpty()) {
        responseOutput->setText("Por favor, primero suelta una imagen.");
        return;
    }
    if (prompt.isEmpty()) {
        responseOutput->setText("Por favor, introduce tu pregunta o la información que deseas obtener.");
        return;
    }
    responseOutput->setText("Contactando a Gemini API...");
    processButton->setEnabled(false);
    resetButton->setEnabled(false); // Deshabilitar también el botón de reinicio mientras se procesa
    callGeminiApi(apiKey, currentImagePath, prompt);
}

// NUEVO SLOT para manejar el clic del botón de reinicio
void AppWidget::onResetButtonClicked()
{
    currentImagePath.clear();        // Limpiar la ruta de la imagen en AppWidget
    dragDropInput->clearDroppedImage(); // Indicar a DragDropArea que se reinicie
    promptLineEdit->clear();         // Limpiar el campo del prompt
    responseOutput->clear();         // Limpiar el área de respuesta

    updateProcessButtonState();      // Actualizar el estado del botón "Generar Descripción"
    processButton->setEnabled(false); // Asegurarse de que esté deshabilitado explícitamente
        // ya que updateProcessButtonState podría no deshabilitarlo si solo
        // se limpió el prompt pero la imagen anterior persistiera en la lógica
        // (aunque currentImagePath.clear() lo maneja).
        // Esta línea es una doble seguridad.
    // Si se deshabilitó el botón de reinicio en processInput, aquí lo podríamos rehabilitar,
    // pero generalmente el botón de reinicio siempre debería estar activo.
    // Si se deshabilita en processInput, asegurarse de rehabilitarlo en onGeminiApiReplyFinished también.
    // Por ahora, lo mantendremos simple y el botón de reinicio siempre estará activo excepto durante la llamada a la API.
}


void AppWidget::callGeminiApi(const QString &apiKey, const QString &imagePath, const QString &promptText)
{
    // ... (contenido de callGeminiApi sin cambios, solo asegúrate de que al final de la respuesta,
    // en onGeminiApiReplyFinished, se rehabilite el botón de reinicio si se deshabilitó)
    QFile imageFile(imagePath);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        responseOutput->setText(QString("Error: No se pudo abrir el archivo de imagen: %1").arg(imageFile.errorString()));
        processButton->setEnabled(true);
        resetButton->setEnabled(true); // Rehabilitar
        updateProcessButtonState();
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
        else {
            responseOutput->setText(QString("Error: Tipo de imagen no soportado o no reconocido: %1").arg(extension));
            processButton->setEnabled(true);
            resetButton->setEnabled(true); // Rehabilitar
            updateProcessButtonState();
            return;
        }
    }

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
    generationConfig["maxOutputTokens"] = 2048;
    QJsonObject requestBodyJson;
    requestBodyJson["contents"] = contentsArray;
    requestBodyJson["generationConfig"] = generationConfig;
    QJsonDocument jsonDoc(requestBodyJson);
    QByteArray postData = jsonDoc.toJson();
    QString apiUrlString = GEMINI_API_ENDPOINT_TEMPLATE.arg(apiKey);
    QUrl apiUrl(apiUrlString);
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = networkManager->post(request, postData);
    connect(reply, SIGNAL(finished()), this, SLOT(onGeminiApiReplyFinished()));
}

void AppWidget::onGeminiApiReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        responseOutput->setText("Error: No se pudo obtener el objeto QNetworkReply del emisor de la señal.");
        processButton->setEnabled(true);
        resetButton->setEnabled(true); // Rehabilitar botón de reinicio
        updateProcessButtonState();
        return;
    }

    processButton->setEnabled(true);
    resetButton->setEnabled(true); // <-- REHABILITAR BOTÓN DE REINICIO
    updateProcessButtonState();

    // ... (resto de la lógica de onGeminiApiReplyFinished sin cambios) ...
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
                    emit descriptionGenerated(generatedText);
                } else {
                    responseOutput->setText("Respuesta de Gemini recibida, pero sin partes de texto.");
                }
            } else {
                responseOutput->setText("Respuesta de Gemini recibida, pero sin candidatos.");
            }
        } else if (jsonObject.contains("error")) {
            QJsonObject errorObj = jsonObject["error"].toObject();
            QString errorMessage = errorObj["message"].toString();
            responseOutput->setText(QString("Error de la API de Gemini: %1").arg(errorMessage));
        } else {
            responseOutput->setText("Respuesta JSON de Gemini no reconocida o vacía.");
        }
    } else {
        QString errorString = QString("Error de red: %1\nRespuesta del servidor (si la hay):\n%2")
        .arg(reply->errorString())
            .arg(QString::fromUtf8(reply->readAll()));
        responseOutput->setText(errorString);
    }
    reply->deleteLater();
}

void AppWidget::displayDescription(const QString &description)
{
    responseOutput->setText(description);
}

void AppWidget::updateProcessButtonState()
{
    bool canProcess = !currentImagePath.isEmpty() &&
                      !promptLineEdit->text().trimmed().isEmpty();
    // El botón de "Generar Descripción" se habilita/deshabilita aquí.
    // El botón de "Reiniciar" usualmente siempre está activo,
    // excepto quizás durante una llamada a la API.
    processButton->setEnabled(canProcess);
}
