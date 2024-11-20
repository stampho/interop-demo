#include <QDebug>
#include <QGuiApplication>
#include <QObject>
#include <QQuickWindow>
#include <rhi/qrhi.h>

#include <QQuickView>
#include <QQmlEngine>
#include <QQmlComponent>

#include "textureitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);

    QQuickWindow win;
    win.resize(800, 600);
    win.setTitle(QStringLiteral("Interop DEMO"));
    win.show();

    QObject::connect(&win, &QQuickWindow::sceneGraphInitialized, [&win]() {
        QRhi *rhi = win.rhi();
        if (!rhi) {
            qWarning("Backend: Software");
            return;
        }

        qDebug("RHI Info:");
        qDebug().noquote() << "    Backend:" << rhi->backendName();
        qDebug().noquote() << "     Device:" << rhi->driverInfo().deviceName;
        qDebug().noquote() << "  Vendor ID:"
                           << QStringLiteral("0x%1").arg(QString::number(rhi->driverInfo().vendorId, 16));
    });

    TextureItem *textureItem = new TextureItem(win.contentItem());

    return app.exec();
}
