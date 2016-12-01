#include <QtQuick>
#include <QtQml>
#include <QtCore>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QQuickRenderControl renderControl;

    QQuickWindow window(&renderControl);
    window.resize(64, 64);
    window.create();

    QOpenGLContext gl;
    gl.setFormat(window.format());
    gl.create();
    bool ok = gl.makeCurrent(&window);
    if (!ok) {
        qDebug() << "failed to GL::MakeCurrent!";
        return 3;
    }

    renderControl.initialize(&gl);

    QQmlEngine engine;
    QQmlComponent component(&engine, QStringLiteral("main.qml"));

    QObject *object = component.create();
    if (component.isError()) {
        qDebug() << "utter and complete failure...";
        qDebug() << component.errors();
        return 1;
    }

    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        qDebug() << "root object is not a QQuickItem";
        return 2;
    }
    item->setParentItem(window.contentItem());

    QOpenGLFramebufferObject fbo(item->width(), item->height(), QOpenGLFramebufferObject::CombinedDepthStencil);
    window.setRenderTarget(&fbo);
    renderControl.sync();
    renderControl.render();
    window.setRenderTarget(nullptr);

    fbo.toImage().save("result.png");

    renderControl.invalidate();

    return 0;
}
