#include "mainwidght.h"
#include <qpainter.h>
#include <qlistwidget>
#include <qpushbutton.h>

#include <QRandomGenerator>

#pragma execution_character_set("utf-8")

MainWidght::MainWidght(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidghtClass())
{
    ui->setupUi(this);

    connect(ui->btn_close, &QPushButton::clicked, this, [=]() {
        this->close();
        });

    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setFixedSize(this->width(), this->height());

    //按钮悬停效果
    ui->btn_close->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");
    ui->btn_about->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");
    ui->btn_skin->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");

    //判断数据库连接是否存在，存在就得到连接，不存在添加到链接
    if (QSqlDatabase::contains("sql_default_connetion")) {
        //根据数据库默认连接名称得到连接
        db = QSqlDatabase::database("sql_default_connetion");
    }
    else {
        db = QSqlDatabase::addDatabase("QSQLITE");//添加数据库，得到默认连接
        db.setDatabaseName("mp3listdatabase.db");//设置数据库文件名
    }

    //打开数据库，打开标识 QSqlQuery类
    if (!db.open()) {
        QMessageBox::critical(0, QObject::tr("Open Data Error."), db.lastError().text());
    }
    else {
        //定义query对象，得到打开的数据库标识
        QSqlQuery query;
        QString sql = "create table if not exists searchlist(id integer,songname text,singername text,url_id text)";
        
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("create searchlist Error."), db.lastError().text());
        }

        //播放历史记录数据表
        sql = "create table if not exists historysong(id integer,songname text,singername text,url_id text)";
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("create historysong Error."), db.lastError().text());
        }

        //查询历史数据表中的插入歌曲数据
        sql = "select *from historysong;";
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("select historysong Error."), db.lastError().text());
        }

        while (query.next()) {
            QString songname, singername;
            QSqlRecord rec = query.record();
            int ablumkey = rec.indexOf("songname");
            int hashkey = rec.indexOf("singername");
            songname = query.value(ablumkey).toString();
            singername = query.value(hashkey).toString();

            QString strshow = songname + "——" + singername;
            QListWidgetItem* item = new QListWidgetItem(strshow);
            ui->lw_history->insertItem(0, item);
        }

        //搜索歌曲
        connect(ui->btn_search, &QPushButton::clicked, this, &MainWidght::Search_clicked);

        //播放操作
        player = new QMediaPlayer;
        playerlist = new QMediaPlaylist;

        //更新播放的进度条
        connect(player, &QMediaPlayer::positionChanged, this, [=](qint64 value) {
            // 获取音频/视频文件的总时长，并设置为滑块的最大值
            ui->Hslider_time->setRange(0, player->duration());
            ui->Hslider_time->setValue(value);
            });

        //显示歌词
        connect(this, &MainWidght::lyricShow, this, &MainWidght::lyricTextShow);

        //双击搜索列表歌曲播放音乐
        connect(ui->lw_search, &QListWidget::doubleClicked, this, &MainWidght::playSearchMusic);
        //双击历史列表歌曲播放音乐
        connect(ui->lw_history, &QListWidget::doubleClicked, this, &MainWidght::playHistoryhMusic);

        num = 0;

    }

    //上一曲
    connect(ui->btn_last, &QPushButton::clicked, this, &MainWidght::lastplay);
    //下一曲
    connect(ui->btn_next, &QPushButton::clicked, this, &MainWidght::nextplay);

    //播放暂停
    connect(ui->btn_play, &QPushButton::clicked, this, [=]() {
        if (player->state() == QMediaPlayer::PlayingState) {
            player->pause();
            ui->btn_play->setIcon(QIcon(":/image/play4.png"));
        }
        else if (player->state() == QMediaPlayer::PausedState) {
            player->play();
            ui->btn_play->setIcon(QIcon(":/image/play1.png"));
        }
        });

    //循环播放
    connect(ui->pushButton_5, &QPushButton::clicked, this, [=]() {
        currentState = (currentState + 1) % 3;
        ui->pushButton_5->setIcon(QIcon(states[currentState]));
        });

    // 连接播放结束的信号到槽函数
    connect(player, &QMediaPlayer::stateChanged, [=](QMediaPlayer::State state) {
        if (state == QMediaPlayer::EndOfMedia) {
            qDebug() << "音频播放结束了！";
            //执行下一曲函数
            MainWidght::nextplay();
        }
        });

    //音量开关


    //音量调整
    connect(ui->Hslider_voice, &QSlider::valueChanged, [=](int value) {
        player->setVolume(value);
        ui->label_voice->setText(QString::number(value));
        });

    //控制进度
    connect(ui->Hslider_time, &QSlider::sliderPressed, [=]() {
        i = false;
        });
    connect(ui->Hslider_time, &QSlider::sliderReleased, [=]() {
        i = true;
        });
    connect(ui->Hslider_time, &QSlider::valueChanged, [=](int value) {
        QTime time(0, value / 60000, qRound((value % 60000) / 1000.0));
        ui->label_time->setText(time.toString("mm:ss"));
        if (i == false) {
            player->setPosition(qint64(value));
        }
        });

    //初始化默认背景
    backgroundtodefault();

    //更换皮肤菜单项
    QAction* actiontoDefault = new QAction();
    actiontoDefault->setText("默认背景");
    actiontoDefault->setIcon(QIcon(":/image/default.png"));

    QAction* actiontoSetting = new QAction();
    actiontoSetting->setText("自定义背景");
    actiontoSetting->setIcon(QIcon(":/image/setting.png"));

    connect(actiontoDefault, &QAction::triggered, this, &MainWidght::backgroundtodefault);
    connect(actiontoSetting, &QAction::triggered, this, &MainWidght::backgroundtosetting);

    menuchangeSkin = new QMenu(this);
    menuchangeSkin->addAction(actiontoDefault);
    menuchangeSkin->addAction(actiontoSetting);

    //更换窗口皮肤功能
    //点击出现菜单项
    connect(ui->btn_skin, &QPushButton::clicked, this, [=]() {
        menuchangeSkin->exec(QCursor::pos());
        });

    painter = new QPainter(this);

    //关于
    connect(ui->btn_about, &QPushButton::clicked, this, [=]() {
        QMessageBox::about(this, "About", "MP3音乐播放器搜索引擎\n\n"
            "作 者：必须小心谨慎");
        });

    //系统托盘初始化
    initSysti();

}

MainWidght::~MainWidght()
{
    delete ui;
}

void MainWidght::paintEvent(QPaintEvent* event)
{
    //QPainter painter(this);
    ////if (pixnum > 4)pixnum = 1;
    ////painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(QString(":/image/qwer%1.png").arg(pixnum++)));
    //painter.drawPixmap(0, 0, this->width(), this->height(), QPixmap(QString(":/image/qwer4.png")));
}

void MainWidght::mouseMoveEvent(QMouseEvent* event)
{
    if (mousePress) {
        this->move(event->globalPos() - movePoint);
        this->setCursor(Qt::ClosedHandCursor);
    }
}

void MainWidght::mouseReleaseEvent(QMouseEvent* event)
{
    if (mousePress) {
        this->setCursor(Qt::ArrowCursor);
        mousePress = false;
    }
}

void MainWidght::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        movePoint = event->globalPos() - this->pos();
        mousePress = true;
    }
}


//读取网络数据
void MainWidght::netReply(QNetworkReply* reply)
{
    //获取响应信息 200为正常
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << status_code;
    //重定向
    reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    //如果没有错误返回
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        emit finish(data);
    }
    else {
        qDebug() << reply->errorString();
    }
    reply->deleteLater();

}

//显示歌词
void MainWidght::lyricTextShow(QString lycris)
{
    ui->textBrowser->setFont(QFont("宋体", 10, QFont::Bold));
    ui->textBrowser->setTextColor(Qt::white);
    ui->textBrowser->setText(lycris);
}

void MainWidght::Search_clicked()
{
    //将原有歌曲数据清空
    ui->lw_search->clear();
    //清理数据库中以及存储的 数据
    QSqlQuery query;
    QString sql = "delete from searchlist;";
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("Delete searchlist Error."), db.lastError().text());
    }

    //根据用户输入的音乐名称，发起请求操作
    QString url = wangyiSearchApi + QString("s=%1&offset=0&page=1&limit=20&type=1").arg(ui->lineEdit->text());
    httpAccess(url);

    QByteArray JsonData;
    QEventLoop loop;

    auto c = connect(this, &MainWidght::finish, [&](const QByteArray& data) {
        JsonData = data;
        loop.exit(1);
        });

    loop.exec();
    disconnect(c);

    //解析网页回复的数据，将搜索得到的音乐url_id的索引值存放到数据库
    url_idJsonAnalysis(JsonData);
}

//播放音乐//搜索列表
void MainWidght::playSearchMusic()
{
    //双击歌曲获得对应索引，即数据库当中数据表的id号
    int row = ui->lw_search->currentRow();
    qDebug() << "row--" << row;

    //查询搜索数据库中的数据表中存储的音乐的数据信息
    QSqlQuery query;
    QString sql = QString("select *from searchlist where id=%1;").arg(row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select searchlist table Error."), db.lastError().text());
    }

    //将选中的音乐的数据信息存入历史数据表中
    QString songname, singername, url_id;
    while (query.next())
    {
        QSqlRecord recd = query.record();
        int songkey = recd.indexOf("songname");
        int singerkey = recd.indexOf("singername");
        int idkey = recd.indexOf("url_id");

        songname = query.value(songkey).toString();
        singername = query.value(singerkey).toString();
        url_id = query.value(idkey).toString();

        sql = QString("select id from historysong where url_id='%1';").arg(url_id);
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("select id Error."), db.lastError().text());
        }
        //将选中的音乐的数据信息存入历史数据表中
        if (query.next() == NULL) {
            sql = QString("insert into historysong values(%1,'%2','%3','%4')").arg(ui->lw_history->count() + 1).arg(songname).arg(singername).arg(url_id);
            if (!query.exec(sql)) {
                QMessageBox::critical(0, QObject::tr("insert historysong Error."), db.lastError().text());
            }

            //将解析的音乐名称保存到lw_history中显示
            QString show = songname + "——" + singername;
            QListWidgetItem* item = new QListWidgetItem(show);
            ui->lw_history->insertItem(0, item);

        }
    }
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);
}

//播放音乐//历史列表
void MainWidght::playHistoryhMusic()
{
    //双击歌曲获得对应索引，即数据库当中数据表的id号
    int row = ui->lw_history->currentRow();
    qDebug() << "row--" << row;
    //查询搜索数据库中的历史记录中存储的音乐数据信息
    QSqlQuery query;
    QString sql = QString("select *from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select historysong table Error."), db.lastError().text());
    }

    //记录该歌曲的信息
    QString songname, singername, url_id;
    while (query.next())
    {
        QSqlRecord recd = query.record();
        int songkey = recd.indexOf("songname");
        int singerkey = recd.indexOf("singername");
        int idkey = recd.indexOf("url_id");

        songname = query.value(songkey).toString();
        singername = query.value(singerkey).toString();
        url_id = query.value(idkey).toString();
    }

    //播放
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);

    //删除更新音乐在数据库中的位置以及lw_history中的位置
    sql = QString("delete from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("delete historysong table Error."), db.lastError().text());
    }
    sql = QString("update historysong set id=id-1 where id>=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("updata historysong table Error."), db.lastError().text());
    }

    //删除在lw_history中的项
    delete ui->lw_history->takeItem(row);

    sql = QString("insert into historysong values(%1,'%2','%3','%4')").arg(ui->lw_history->count() + 1).arg(songname).arg(singername).arg(url_id);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("insert historysong Error."), db.lastError().text());
    }

    //将解析的音乐名称保存到lw_history中显示
    QString show = songname + "——" + singername;
    QListWidgetItem* item = new QListWidgetItem(show);
    ui->lw_history->insertItem(0, item);

    this->row = 0;

}

//上一曲
void MainWidght::lastplay()
{
    row++;
    if (row > ui->lw_history->count() - 1) {
        row = 0;
    }

    //查询数据库历史记录表中存储的音乐信息
    QSqlQuery query;
    QString sql = QString("select *from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select historysong table Error."), db.lastError().text());
    }

    //将选中的音乐的数据信息存入历史数据表中
    QString url_id;
    while (query.next())
    {
        QSqlRecord recd = query.record();

        int idkey = recd.indexOf("url_id");

        url_id = query.value(idkey).toString();

    }
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);
}
//下一曲
void MainWidght::nextplay()
{
    //检查播放方式
    switch (currentState)
    {
    case 0://循环播放
        row--;
        if (row < 0) {
            row = ui->lw_history->count() - 1;
        }
        break;
    case 1://随机播放
        row = QRandomGenerator::global()->bounded(ui->lw_history->count());
        break;
    case 2://单曲循环
        break;
    default:
        break;
    }

    //查询数据库历史记录表中存储的音乐信息
    QSqlQuery query;
    QString sql = QString("select *from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select historysong table Error."), db.lastError().text());
    }

    QString url_id;
    while (query.next())
    {
        QSqlRecord recd = query.record();

        int idkey = recd.indexOf("url_id");

        url_id = query.value(idkey).toString();

    }
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);
}

//更换默认背景
void MainWidght::backgroundtodefault()
{
    //获取widget的palette
    QPalette palette = this->palette();
    if (pixnum > 4)pixnum = 1;
    palette.setBrush(QPalette::Window,
        QBrush(QPixmap(QString(":/image/qwer%1.png").arg(pixnum++)).scaled(//维族背景图
            this->size(), Qt::IgnoreAspectRatio,//不限制原图片的长宽比例
            Qt::SmoothTransformation)));//使用平滑的缩放方式
    this->setPalette(palette);

    //if (pixnum > 4)pixnum = 1;
    //painter->drawPixmap(0, 0, this->width(), this->height(), QPixmap(QString(":/image/qwer%1.png").arg(pixnum++)));
}
//用户自定义背景
void MainWidght::backgroundtosetting()
{
    //选择打开图片作为背景皮肤
    QString strFileName = QFileDialog::getOpenFileName(this, "请选择自定义背景图片",
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(), u8"图片(*.jpg *.png)");

    //获取widget的palette
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window,
        QBrush(QPixmap(strFileName).scaled(//维族背景图
            this->size(), Qt::IgnoreAspectRatio,//不限制原图片的长宽比例
            Qt::SmoothTransformation)));//使用平滑的缩放方式
    this->setPalette(palette);
}


//下载并播放
void MainWidght::downloadPlayer(QString url_id)
{
    QString mp3url = wangyiDownloadApi + QString("id=%1.mp3").arg(url_id);

    httpAccess(mp3url);

    //QByteArray JsonData;
    QEventLoop loop;

    auto d = connect(this, &MainWidght::finish, [&](const QByteArray& data) {
        //JsonData = data;
        loop.exit(1);
        });

    loop.exec();
    disconnect(d);

    //解析将要播放的音乐
    player->setMedia(QUrl(mp3url));

    //设置音量
    player->setVolume(ui->Hslider_voice->value());
    ui->label_voice->setText(QString::number(ui->Hslider_voice->value()));

    ////设置音量滑动条
    //ui->Hslider_voice->setValue(100);

    //播放音乐
    player->play(); 
    ui->btn_play->setIcon(QIcon(":/image/play1.png"));
    
}

//访问http网页
void MainWidght::httpAccess(QString url)
{
    //实例化网络请求操作事项
    request = new QNetworkRequest;
    //将url网页地址存入request中
    request->setUrl(url);
    //实例化网络管理

    //设置请求头，免登录
    request->setRawHeader("Cookie", "NMTID=00OtdYjju0yepeluUtFq2gWtCBaToEAAAGRBMz92g");
    request->setRawHeader("User-Agent", "PostmanRuntime/7.40.0");

    manager = new QNetworkAccessManager;
    //通过get方法，上传具体请求
    manager->get(*request);

    //当网页回复消息时，触发finished信号，
    connect(manager, &QNetworkAccessManager::finished, this, &MainWidght::netReply);


}

//音乐的url_id解析以及搜索列表显示
void MainWidght::url_idJsonAnalysis(QByteArray JsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(JsonData);
    if (doc.isObject()) {
        QJsonObject data = doc.object();

        if (data.contains("result")) {
            QJsonObject objectInfo = data.value("result").toObject();
            if (objectInfo.contains("songs")) {
                QJsonArray objectSongs = objectInfo.value("songs").toArray();
                int k = objectSongs.count();
                for (int i = 0; i < objectSongs.count(); i++)
                {
                    QString songname, singername, url_id;
                    QJsonObject song = objectSongs.at(i).toObject();

                    if (song.contains("name")) {
                        songname = song.value("name").toString();
                    }

                    if (song.contains("id")) {
                        url_id = QString::number(song.value("id").toInt());
                    }

                    if (song.contains("artists")) {
                        QJsonArray arrayarts =  song.value("artists").toArray();
                        QJsonObject artist = arrayarts.at(0).toObject();
                        if (artist.contains("name")) {
                            singername = artist.value("name").toString();
                        }
                    }

                    QSqlQuery query;
                    QString sql = QString("insert into searchlist values(%1,'%2','%3','%4')").arg(QString::number(i)).arg(songname).arg(singername).arg(url_id);
                    if (!query.exec(sql)) {
                        QMessageBox::critical(0, QObject::tr("insert searchlist Error."), db.lastError().text());
                    }

                    //将解析的音乐名称存入lw_search进行显示
                    QString show = songname + "——" + singername;
                    QListWidgetItem* item = new QListWidgetItem(show);
                    ui->lw_search->addItem(item);

                }

            }
        }
    }

    if (doc.isArray()) {
        qDebug() << "Array";
    }
}

//歌词解析
void MainWidght::lyricJsonAnalysis(QString url_id)
{
    //根据用户输入的音乐名称，发起请求操作
    QString url = wangyilyricsApi + QString("id=%1").arg(url_id);
    httpAccess(url);

    QByteArray JsonData;
    QEventLoop loop;

    auto c = connect(this, &MainWidght::finish, [&](const QByteArray& data) {
        JsonData = data;
        loop.exit(1);
        });

    loop.exec();
    disconnect(c);

    //解析网页回复的歌词
    QJsonDocument doc = QJsonDocument::fromJson(JsonData);
    if (doc.isObject()) {
        QJsonObject data = doc.object();
        if (data.contains("lyric")) {
            emit lyricShow(data.value("lyric").toString());
            
        }
        else {
            emit lyricShow("暂无歌词");
        }
        if (doc.isArray()) {
            qDebug() << "Array";
        }
    }
}

//响应系统托盘的双击操作
void MainWidght::SystiActicated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        //显示隐藏界面
        if (isHidden()) {
            show();
        }
        else {
            hide();
        }
        break;

    default:
        break;
    }

}
//系统托盘初始化
void MainWidght::initSysti()
{
    mySystemTray = new QSystemTrayIcon(this);
    mySystemTray->setIcon(QIcon(":/image/qwer.png"));
    connect(mySystemTray, &QSystemTrayIcon::activated, this, &MainWidght::SystiActicated);

    QAction* actionSysquit = new QAction(QIcon(":/image/quit.png"),u8"退出程序");
    connect(actionSysquit, &QAction::triggered, this, &MainWidght::quitmusicplayer);

    QMenu* pcontextmenu = new QMenu(this);
    pcontextmenu->addAction(actionSysquit);
    mySystemTray->setContextMenu(pcontextmenu);
    mySystemTray->show();

}
//托盘退出程序
void MainWidght::quitmusicplayer()
{
    QCoreApplication::quit();

}
