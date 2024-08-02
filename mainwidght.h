#pragma once

#include <QtWidgets/QWidget>
#include "ui_mainwidght.h"
#include <QNetworkRequest>//http��url�������
#include <QNetworkAccessManager>//URL���ϴ�����
#include <QNetworkReply>//��ҳ�ظ����ݴ����ź���
#include <QEventLoop>//QEventLoop�ṩһ�ֽ�����뿪�¼�ѭ���ķ���
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QTime>
#include <QMessageBox>
#include <math.h>

#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QStandardPaths>

//ϵͳ����ͷ�ļ�
#include <QSystemTrayIcon>


//����������ȡID
static QString wangyiSearchApi = "https://music.163.com/api/search/pc?";
//ͨ������ID���ظ���
static QString wangyiDownloadApi = "http://music.163.com/song/media/outer/url?";
//ͨ������ID���ظ��
static QString wangyilyricsApi = "http://music.163.com/api/song/media?";

static int pixnum = 1;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidghtClass; };
QT_END_NAMESPACE

class MainWidght : public QWidget
{
    Q_OBJECT

public:
    MainWidght(QWidget *parent = nullptr);
    ~MainWidght();

    void paintEvent(QPaintEvent* event);


    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);


signals:
    void finish(QByteArray);
    void lyricShow(QString);

private slots:

    //��ȡ�������ݵĲۺ���
    void netReply(QNetworkReply* reply);

    //��ʾ��ʵĲۺ���
    void lyricTextShow(QString lycris);

    //��������
    void Search_clicked();

    //��������
    void playSearchMusic();//�����б�
    void playHistoryhMusic();//��ʷ�б�

    //��һ����һ��
    void lastplay();
    void nextplay();

    //����Ƥ������
    void backgroundtodefault();//����Ĭ�ϱ���
    void backgroundtosetting();//�û��Զ��屳��

protected:
    //���ָ������غͲ���
    void downloadPlayer(QString url_id);
    
    //����HTTP��ҳ
    void httpAccess(QString url);

    //���ֵ�hash��ablum_idֵ������ʹ��JSON
    void url_idJsonAnalysis(QByteArray);

    ////��������������ϢJSON����
    //QString musicJsonAnalysis(QByteArray JsonData);

    //��ʽ���
    void lyricJsonAnalysis(QString url_id);

    //



private:
    Ui::MainWidghtClass *ui;

    QNetworkRequest* request;
    QNetworkAccessManager* manager;
    QMediaPlayer* player;
    QMediaPlaylist* playerlist;
    QSqlDatabase db;
    int num;
    int row = 0;
    bool i = true;

    //ѭ������ö��ֵ
    int currentState = 0;
    QStringList states = { ":/image/sequence.png",":/image/shuffle.png",":/image/update.png" };

    //��������϶������ƶ�
    bool mousePress = false;
    QPoint movePoint;//ƫ��ֵ
    

    QPainter *painter;
    //����Ƥ���˵�
    QMenu* menuchangeSkin;

    //ϵͳ����
    QSystemTrayIcon* mySystemTray;

    //ϵͳ���̳�ʼ��
    void initSysti();

private slots:
    //��Ӧϵͳ���̵�˫������
    void SystiActicated(QSystemTrayIcon::ActivationReason reason);
    //�˳�����
    void quitmusicplayer();
};
