#ifndef TEXTUREITEM_H
#define TEXTUREITEM_H

#include <QQuickItem>

class TextureItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
public:
    TextureItem(QQuickItem *parent = nullptr);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private slots:
    void onSizeChanged();
};

#endif // TEXTUREITEM_H
