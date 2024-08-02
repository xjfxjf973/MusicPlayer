#pragma once

#include <QtWidgets/QWidget>
#include "ui_mainwidght.h"
#include <QNetworkRequest>//http的url请求管理
#include <QNetworkAccessManager>//URL的上传管理
#include <QNetworkReply>//网页回复数据触发信号类
#include <QEventLoop>//QEventLoop提供一种进入和离开事件循环的方法
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

//系统托盘头文件
#include <QSystemTrayIcon>


//搜索歌曲获取ID
static QString wangyiSearchApi = "https://music.163.com/api/search/pc?";
//通过歌曲ID下载歌曲
static QString wangyiDownloadApi = "http://music.163.com/song/media/outer/url?";
//通过歌曲ID下载歌词
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

    //读取网络数据的槽函数
    void netReply(QNetworkReply* reply);

    //显示歌词的槽函数
    void lyricTextShow(QString lycris);

    //搜索歌曲
    void Search_clicked();

    //播放音乐
    void playSearchMusic();//搜索列表
    void playHistoryhMusic();//历史列表

    //上一曲下一曲
    void lastplay();
    void nextplay();

    //更换皮肤功能
    void backgroundtodefault();//更换默认背景
    void backgroundtosetting();//用户自定义背景

protected:
    //音乐歌曲下载和播放
    void downloadPlayer(QString url_id);
    
    //访问HTTP网页
    void httpAccess(QString url);

    //音乐的hash和ablum_id值解析，使用JSON
    void url_idJsonAnalysis(QByteArray);

    ////搜索音乐数据信息JSON解析
    //QString musicJsonAnalysis(QByteArray JsonData);

    //歌词解析
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

    //循环播放枚举值
    int currentState = 0;
    QStringList states = { ":/image/sequence.png",":/image/shuffle.png",":/image/update.png" };

    //处理鼠标拖动窗口移动
    bool mousePress = false;
    QPoint movePoint;//偏移值
    

    QPainter *painter;
    //更换皮肤菜单
    QMenu* menuchangeSkin;

    //系统托盘
    QSystemTrayIcon* mySystemTray;

    //系统托盘初始化
    void initSysti();

private slots:
    //响应系统托盘的双击操作
    void SystiActicated(QSystemTrayIcon::ActivationReason reason);
    //退出程序
    void quitmusicplayer();
};
