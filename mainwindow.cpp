#include "mainwindow.h"
#include "ui_mainwindow.h"




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    vlayOut=ui->tab->layout();
    ui->label_3->setStyleSheet("font-size:60px;");
    ui->label_8->setStyleSheet("font-size:10px;");
    ui->label_11->setStyleSheet("font-size:10px;");
    ui->horizontalSlider->setMaximum(this->frameGeometry().height()-50);
    ui->horizontalSlider->setMinimum(20);
    QSize spacerSize= ui->verticalSpacer->sizeHint();
    ui->horizontalSlider->setValue(spacerSize.height());
    connect(ui->horizontalSlider, &QSlider::valueChanged, this, &MainWindow::slot_changeSpeedDialPosition);

    ui->horizontalSpacer_4->changeSize(ui->verticalLayout->geometry().width(),ui->horizontalSpacer_4->geometry().height());
    vlayOut->invalidate();

    connect(inUseSatellitesTimer, &QTimer::timeout, this, &MainWindow::slot_inUseSatellitesTimeOut);
    connect(inViewSatellitesTimer, &QTimer::timeout, this, &MainWindow::slot_inViewSatellitesTimeOut);


    QGeoPositionInfoSource *Positionsource = QGeoPositionInfoSource::createDefaultSource(this);
    if (Positionsource)
    {
        connect(Positionsource, SIGNAL(positionUpdated(QGeoPositionInfo)),this, SLOT(solt_positionUpdated(QGeoPositionInfo)));
        Positionsource->setUpdateInterval(POSITION_UPDATE);
        Positionsource->startUpdates();
    }

    QGeoSatelliteInfoSource *satelliteSource= QGeoSatelliteInfoSource::createDefaultSource(this);
    if(satelliteSource)
    {
        connect(satelliteSource, &QGeoSatelliteInfoSource::satellitesInViewUpdated, this,&MainWindow::solt_satellitesInViewUpdated);
        connect(satelliteSource, &QGeoSatelliteInfoSource::satellitesInUseUpdated, this,&MainWindow::solt_satellitesInUseUpdated);
        satelliteSource->setUpdateInterval(0);
        satelliteSource->startUpdates();
    }






    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([]() {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        // Hide system ui elements or go full screen
        activity.callObjectMethod("getWindow", "()Landroid/view/Window;")
            .callObjectMethod("getDecorView", "()Landroid/view/View;")
            .callMethod<void>("setSystemUiVisibility", "(I)V", 0xffffffff);
        activity.callObjectMethod("getWindow", "()Landroid/view/Window;")
            .callMethod<void>("addFlags", "(I)V", 0x00000080);
    });



}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::solt_positionUpdated(const QGeoPositionInfo &info)
{
    double nowLongitude = info.coordinate().longitude();
    double nowLatitude = info.coordinate().latitude();
    double nowGroundSpeed = info.attribute(QGeoPositionInfo::GroundSpeed);
    double nowVerticalSpeed = info.attribute(QGeoPositionInfo::VerticalSpeed);
    double nowMagneticVariation = info.attribute(QGeoPositionInfo::MagneticVariation);
    double nowHorizontalAccuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    double nowVerticalAccuracy = info.attribute(QGeoPositionInfo::VerticalAccuracy);
    QString nowTimes = info.timestamp().toString();
    double GForce=(nowGroundSpeed-lastGSpeed)/GForceTimer->elapsed()*1000/9.8;
    ui->label_17->setText(QString("G值:")+QString::number(GForce, 'f', 7));
    lastGSpeed=nowGroundSpeed;
    if(GForce>=0)
    {
        if(GForce>maxPositiveGForece)
        {
        maxPositiveGForece=GForce;
        ui->label_18->setText(QString("最大正G值:")+QString::number(maxPositiveGForece, 'f', 7));
        }
    }
    else
    {
        if(GForce<maxnegativeGForece)
        {
        maxnegativeGForece=GForce;
        ui->label_20->setText(QString("最大负G值:")+QString::number(maxnegativeGForece, 'f', 7));
        }
    }
    GForceTimer->restart();


    if(nowGroundSpeed*3.6>maxSpeed)
    {
        maxSpeed= nowGroundSpeed*3.6;
        ui->label_16->setText(QString("最大速度:")+QString::number(maxSpeed, 'f', 2));
    }
    ui->label->setText( QString("经度:") +QString::number(nowLongitude, 'f', 7));
    ui->label_2->setText( QString("维度:")+QString::number(nowLatitude, 'f', 7));
    if(isFirstGetPosition==0)
    {
        double shortDistance=realDistance(lastLat,lastLng,nowLatitude,nowLongitude);
        if(nowHorizontalAccuracy<5&&!isnan(nowGroundSpeed))
        {
            tripDistance+=shortDistance;
            ui->label_19->setText( QString("小计里程:")+QString::number(tripDistance, 'f', 2));
            lastLat=nowLatitude;
            lastLng=nowLongitude;
        }

    }
    else
    {
        lastLat=nowLatitude;
        lastLng=nowLongitude;
        isFirstGetPosition=0;
    }
    ui->label_3->setText( QString::number(nowGroundSpeed*3.6, 'f', 2));
    ui->label_4->setText( QString("垂直速度:")+QString::number(nowVerticalSpeed, 'f', 7));
    ui->label_5->setText( QString("磁偏角:")+QString::number(nowMagneticVariation, 'f', 7));
    ui->label_6->setText( QString("水平精度:")+QString::number(nowHorizontalAccuracy, 'f', 7));
    ui->label_7->setText( QString("垂直精度:")+QString::number(nowVerticalAccuracy, 'f', 7));
    ui->label_8->setText(QString("时间:")+nowTimes);
    double validSpeed;
    if(isnan(nowGroundSpeed))
    {
        validSpeed=0;
    }
    else
    {
        validSpeed=nowGroundSpeed;
    }
    ui->label_21->setText( QString("平均速度:")+QString::number((validSpeed+nowGroundSpeed)/2, 'f', 2));

    ui->horizontalSpacer_4->changeSize(ui->verticalLayout->geometry().width(),ui->horizontalSpacer_4->geometry().height());
    vlayOut->invalidate();

}

void MainWindow::solt_satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    inUseSatellitesTimer->stop();
    QString qsSatelliteSystem("在用卫星种类:");
    QString qsSignalStrength("在用信号强度:");
    QString qsSatelliteNum("在用卫星数量:");
    double allSignalStrength=0;
    int satelliteNum=0;
    foreach (QGeoSatelliteInfo satellite, satellites)
    {
        satelliteNum++;
        qsSatelliteSystem+=QString::number(satellite.satelliteSystem())+QString(" ");
        qsSignalStrength+=QString::number(satellite.signalStrength())+QString(" ");
        allSignalStrength+=satellite.signalStrength();
    }
    ui->label_9->setText(qsSatelliteSystem);
    ui->label_10->setText(qsSignalStrength);
    ui->label_11->setText(QString("信号强度:")+QString::number(allSignalStrength/satelliteNum, 'f', 7));
    ui->label_12->setText(qsSatelliteNum+QString::number(satelliteNum));

    inUseSatellitesTimer->start(2000);
}

void MainWindow::solt_satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    inViewSatellitesTimer->stop();
    QString qsSatelliteSystem("可见卫星种类:");
    QString qsSignalStrength("可见信号强度:");
    QString qsSatelliteNum("可见卫星数量:");
     int satelliteNum=0;
    foreach (QGeoSatelliteInfo satellite, satellites)
    {
        satelliteNum++;
        qsSatelliteSystem+=QString::number(satellite.satelliteSystem())+QString(" ");
        qsSignalStrength+=QString::number(satellite.signalStrength())+QString(" ");
    }
    ui->label_14->setText(qsSatelliteSystem);
    ui->label_15->setText(qsSignalStrength);
    ui->label_13->setText(qsSatelliteNum+QString::number(satelliteNum));

    inViewSatellitesTimer->start(2000);
}

void MainWindow::slot_inUseSatellitesTimeOut()
{
    ui->label_11->setText(QString("信号强度:0"));
    ui->label_9->setText(QString("在用卫星种类:nan"));
    ui->label_10->setText(QString("在用信号强度:nan"));
    ui->label_12->setText(QString("在用卫星数量:nan"));
    inUseSatellitesTimer->stop();
}

void MainWindow::slot_inViewSatellitesTimeOut()
{
    ui->label_14->setText(QString("在用卫星种类:nan"));
    ui->label_15->setText(QString("在用信号强度:nan"));
    ui->label_13->setText(QString("在用卫星数量:nan"));
    inViewSatellitesTimer->stop();
}

void MainWindow::slot_changeSpeedDialPosition(int value)
{
    QSize spacerSize= ui->verticalSpacer->sizeHint();
    ui->verticalSpacer->changeSize(spacerSize.width(),value);
    vlayOut->invalidate();
}

double MainWindow::realDistance(double lat1,double lng1,double lat2,double lng2)
{

    double a;
    double b;
    double radLat1 = rad(lat1);
    double radLat2 = rad(lat2);
    a = radLat1 - radLat2;
    b = rad(lng1) - rad(lng2);
    double s = 2 * asin(sqrt(pow(sin(a/2),2) + cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
    s = s * EARTH_RADIUS;
    s = s * 1000;
    return s;
}

double MainWindow::rad(double d)
{
    return d * pi /180.0;
}

