#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "qgeopositioninfosource.h"
#include"QGeoSatelliteInfoSource.h"
#include<QTimer>
#include <QCoreApplication>
#include <QJniObject>
#include<QVBoxLayout>

#define pi 3.1415926535897932384626433832795
#define EARTH_RADIUS 6378.137
#define POSITION_UPDATE 200


#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLayout  *vlayOut;
    QTimer *inUseSatellitesTimer = new QTimer(this);
    QTimer *inViewSatellitesTimer = new QTimer(this);
    QElapsedTimer *GForceTimer= new QElapsedTimer();
    double maxSpeed=0;
    double lastGSpeed=0;
    double maxPositiveGForece=0;
    double maxnegativeGForece=0;
    double tripDistance=0;
    double lastLat=0;
    double lastLng=0;
    double averageSpeed=0;
    bool isFirstGetPosition=1;
private:
    double realDistance(double lat1,double lng1,double lat2,double lng2);
    double rad(double d);
private slots:
    void solt_positionUpdated(const QGeoPositionInfo &info);
    void solt_satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites);
    void solt_satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &satellites);
    void slot_inUseSatellitesTimeOut();
    void slot_inViewSatellitesTimeOut();
    void slot_changeSpeedDialPosition(int value);
};
#endif // MAINWINDOW_H
