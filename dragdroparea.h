#ifndef DRAGDROPAREA_H
#define DRAGDROPAREA_H

#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QMimeData>
#include <QUrl>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QDragLeaveEvent;
class QPaintEvent;
class QVBoxLayout;

class DragDropArea : public QWidget
{
    Q_OBJECT

public:
    explicit DragDropArea(QWidget *parent = nullptr);

public slots:
    void clearDroppedImage();

signals:
    void imageDropped(const QString &filePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *infoLabel;
    QImage previewImage;
    QString currentFilePathDropped;
};

#endif // DRAGDROPAREA_H
