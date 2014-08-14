#include "AutoGPS.h"
#include <QApplication>
#include <ArcGISLocalTiledLayer.h>
//#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QMessageBox>
#include <QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QGraphicsWidget>
//#include <MapGraphicsView.h>

static const QString UI_OVERLAY_PATH("qrc:/Resources/qml/MainOverlay.qml");

AutoGPS::AutoGPS (QWidget *parent):
    QMainWindow(parent),
    mapGraphicsView(NULL),
    mapController(NULL),
    engine(NULL),
    overlayWidget(NULL),
    overlayUI(NULL),
    mainMenuUI(NULL)
{

    this->setWindowFlags(Qt::CustomizeWindowHint);
    this->showMaximized();
    EsriRuntimeQt::ArcGISRuntime::setRenderEngine(EsriRuntimeQt::RenderEngine::OpenGL);

    mapGraphicsView = EsriRuntimeQt::MapGraphicsView::create(map, this);
    map.setWrapAroundEnabled(false);
    map.setEsriLogoVisible(false);
    this->setCentralWidget(mapGraphicsView);
    QString tpkPath = QApplication::applicationDirPath() + QDir::separator() + "AutoGPS.tpk";
    ArcGISLocalTiledLayer tiledLayer(tpkPath);
    map.addLayer(tiledLayer);

    mapController = new MapController(&map, mapGraphicsView, this);
    engine = new QDeclarativeEngine(this);
    context = new QDeclarativeContext(engine->rootContext());
      context->setContextProperty("serialPortThread", &thread);
//      connect(&thread, SIGNAL(portListChanged), this, SLOT(onPortListChanged));
//      qDebug()<<"portList()"<<QVariant::fromValue(thread.portList());
//      context->setContextProperty("serialPortNo", QVariant::fromValue(thread.portList()));
    QDeclarativeComponent component(engine, QUrl(UI_OVERLAY_PATH), engine);
    overlayUI = component.create(context);
//    thread.setContext(context);
    if (!overlayUI)
    {
        qDebug() << "Failed to load UI overlay";
        qDebug() << component.errorString();
        QMessageBox box;
        box.setText("Error:\n" + component.errorString());
        box.exec();
        return;
    }



    // Create a container for the UML UI and add it to the scene
    overlayWidget = new QGraphicsWidget();
    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(overlayWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    QGraphicsLayoutItem* qmlUILayout = qobject_cast<QGraphicsLayoutItem*>(overlayUI);
    layout->addItem(qmlUILayout);
    overlayWidget->setLayout(layout);
    mapGraphicsView->scene()->addItem(overlayWidget);

    connect(overlayUI, SIGNAL(homeClicked()), mapController, SLOT(handleHomeClicked()));
    connect(overlayUI, SIGNAL(zoomInClicked()), mapController, SLOT(handleZoomIn()));
    connect(overlayUI, SIGNAL(zoomOutClicked()), mapController, SLOT(handleZoomOut()));
    connect(overlayUI, SIGNAL(panClicked(QString)), mapController, SLOT(handlePan(QString)));
    connect(mapController, SIGNAL(positionChanged(QVariant)), overlayUI, SLOT(updateLocation(QVariant)));
    connect(mapController, SIGNAL(speedChanged(QVariant)), overlayUI, SLOT(updateSpeed(QVariant)));
    connect(mapController, SIGNAL(headingChanged(QVariant)), overlayUI, SLOT(updateHeading(QVariant)));

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNorthArrow()));
    timer->start(1000 / 24);
    mainMenuUI = overlayUI->findChild<QObject*>("mainMenu");
    if (mainMenuUI)
    {
        connect(mainMenuUI, SIGNAL(exitClicked()), this, SLOT(close()));
    }

    QObject * serialConfig = overlayUI->findChild<QObject*>("serialConfig");
    if (serialConfig)
    {
        connect(serialConfig, SIGNAL(readyOpenSerialPort(QVariant)), &thread, SLOT(onReadyOpenSerialPort(QVariant)));
    }
}

AutoGPS::~AutoGPS()
{
}

void AutoGPS::updateNorthArrow()
{
    QMetaObject::invokeMethod(overlayUI, "updateMapRotation", Q_ARG(QVariant, map.rotation()));
}

void AutoGPS::resizeEvent(QResizeEvent* event)
{
    if (map.isInitialized())
    {
        QRectF sceneRect = mapGraphicsView->sceneRect();
        overlayWidget->setGeometry(sceneRect);
    }
    QMainWindow::resizeEvent(event);
}
