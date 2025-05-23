#include "appwidget.h"
#include "dragdroparea.h"
#include "datamanager.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>

AppWidget::AppWidget(QWidget *parent)
    : QWidget(parent)
{

    dataManager = new DataManager(this);

    dragDropInput = new DragDropArea(this);

    promptLabel = new QLabel("¿Qué información deseas obtener de la imagen?:", this);
    promptLineEdit = new QLineEdit(this);
    promptLineEdit->setPlaceholderText("Ej: ¿Qué objetos hay? Describe los colores.");

    processButton = new QPushButton("Generar Descripción", this);
    processButton->setEnabled(false);

    resetButton = new QPushButton("Reiniciar Campos", this);

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

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(processButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);

    mainLayout->addWidget(responseLabel);
    mainLayout->addWidget(responseOutput);

    setLayout(mainLayout);

    // Conexiones existentes
    connect(dragDropInput, SIGNAL(imageDropped(const QString&)), this, SLOT(handleImageDropped(const QString&)));
    connect(processButton, SIGNAL(clicked()), this, SLOT(processInput()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetButtonClicked()));
    connect(this, SIGNAL(descriptionGenerated(const QString&)), this, SLOT(displayDescription(const QString&)));
    connect(promptLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProcessButtonState()));

    // NUEVAS CONEXIONES: Conectar señales de DataManager a slots de AppWidget
    connect(dataManager, SIGNAL(processingStarted()), this, SLOT(handleProcessingStarted()));
    connect(dataManager, SIGNAL(descriptionReady(const QString&)), this, SLOT(handleDescriptionReady(const QString&)));
    connect(dataManager, SIGNAL(errorOccurred(const QString&)), this, SLOT(handleErrorOccurred(const QString&)));
    connect(dataManager, SIGNAL(processingFinished()), this, SLOT(handleProcessingFinished()));


    setWindowTitle("Visor de Imágenes con Gemini API (Uso Privado)");
    resize(550, 650);
}

AppWidget::~AppWidget()
{

}

void AppWidget::handleImageDropped(const QString &filePath)
{
    currentImagePath = filePath;
    responseOutput->clear();
    QFileInfo fileInfo(filePath);
    responseOutput->append(QString("Imagen '%1' lista para procesar.").arg(fileInfo.fileName()));
    updateProcessButtonState();
}

void AppWidget::processInput()
{

    QString prompt = promptLineEdit->text().trimmed();
    if (currentImagePath.isEmpty()) {
        responseOutput->setText("Por favor, primero suelta una imagen.");
        return;
    }
    if (prompt.isEmpty()) {
        responseOutput->setText("Por favor, introduce tu pregunta o la información que deseas obtener.");
        return;
    }


    dataManager->fetchDescriptionFromGemini(currentImagePath, prompt);
}

void AppWidget::onResetButtonClicked()
{
    currentImagePath.clear();
    dragDropInput->clearDroppedImage();
    promptLineEdit->clear();
    responseOutput->clear();
    updateProcessButtonState();
}

void AppWidget::handleProcessingStarted()
{
    responseOutput->setText("Contactando a Gemini API...");
    processButton->setEnabled(false);
    resetButton->setEnabled(false); // Deshabilitar también el botón de reinicio mientras se procesa
}

void AppWidget::handleDescriptionReady(const QString &description)
{
    displayDescription(description);
}

void AppWidget::handleErrorOccurred(const QString &errorMessage)
{
    responseOutput->setText(errorMessage);
}

void AppWidget::handleProcessingFinished()
{

    resetButton->setEnabled(true);
    updateProcessButtonState();
}

void AppWidget::displayDescription(const QString &description)
{
    responseOutput->setText(description);
}

void AppWidget::updateProcessButtonState()
{
    bool canProcess = !currentImagePath.isEmpty() &&
                      !promptLineEdit->text().trimmed().isEmpty();

    if (resetButton->isEnabled()) {
        processButton->setEnabled(canProcess);
    }

}
