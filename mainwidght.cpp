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

    //��ť��ͣЧ��
    ui->btn_close->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");
    ui->btn_about->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");
    ui->btn_skin->setStyleSheet("QPushButton:hover{background-color:rgba(255, 255, 255,100);}"
                                "QPushButton{ background-color:rgba(0, 0, 0,0);}");

    //�ж����ݿ������Ƿ���ڣ����ھ͵õ����ӣ���������ӵ�����
    if (QSqlDatabase::contains("sql_default_connetion")) {
        //�������ݿ�Ĭ���������Ƶõ�����
        db = QSqlDatabase::database("sql_default_connetion");
    }
    else {
        db = QSqlDatabase::addDatabase("QSQLITE");//������ݿ⣬�õ�Ĭ������
        db.setDatabaseName("mp3listdatabase.db");//�������ݿ��ļ���
    }

    //�����ݿ⣬�򿪱�ʶ QSqlQuery��
    if (!db.open()) {
        QMessageBox::critical(0, QObject::tr("Open Data Error."), db.lastError().text());
    }
    else {
        //����query���󣬵õ��򿪵����ݿ��ʶ
        QSqlQuery query;
        QString sql = "create table if not exists searchlist(id integer,songname text,singername text,url_id text)";
        
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("create searchlist Error."), db.lastError().text());
        }

        //������ʷ��¼���ݱ�
        sql = "create table if not exists historysong(id integer,songname text,singername text,url_id text)";
        if (!query.exec(sql)) {
            QMessageBox::critical(0, QObject::tr("create historysong Error."), db.lastError().text());
        }

        //��ѯ��ʷ���ݱ��еĲ����������
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

            QString strshow = songname + "����" + singername;
            QListWidgetItem* item = new QListWidgetItem(strshow);
            ui->lw_history->insertItem(0, item);
        }

        //��������
        connect(ui->btn_search, &QPushButton::clicked, this, &MainWidght::Search_clicked);

        //���Ų���
        player = new QMediaPlayer;
        playerlist = new QMediaPlaylist;

        //���²��ŵĽ�����
        connect(player, &QMediaPlayer::positionChanged, this, [=](qint64 value) {
            // ��ȡ��Ƶ/��Ƶ�ļ�����ʱ����������Ϊ��������ֵ
            ui->Hslider_time->setRange(0, player->duration());
            ui->Hslider_time->setValue(value);
            });

        //��ʾ���
        connect(this, &MainWidght::lyricShow, this, &MainWidght::lyricTextShow);

        //˫�������б������������
        connect(ui->lw_search, &QListWidget::doubleClicked, this, &MainWidght::playSearchMusic);
        //˫����ʷ�б������������
        connect(ui->lw_history, &QListWidget::doubleClicked, this, &MainWidght::playHistoryhMusic);

        num = 0;

    }

    //��һ��
    connect(ui->btn_last, &QPushButton::clicked, this, &MainWidght::lastplay);
    //��һ��
    connect(ui->btn_next, &QPushButton::clicked, this, &MainWidght::nextplay);

    //������ͣ
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

    //ѭ������
    connect(ui->pushButton_5, &QPushButton::clicked, this, [=]() {
        currentState = (currentState + 1) % 3;
        ui->pushButton_5->setIcon(QIcon(states[currentState]));
        });

    // ���Ӳ��Ž������źŵ��ۺ���
    connect(player, &QMediaPlayer::stateChanged, [=](QMediaPlayer::State state) {
        if (state == QMediaPlayer::EndOfMedia) {
            qDebug() << "��Ƶ���Ž����ˣ�";
            //ִ����һ������
            MainWidght::nextplay();
        }
        });

    //��������


    //��������
    connect(ui->Hslider_voice, &QSlider::valueChanged, [=](int value) {
        player->setVolume(value);
        ui->label_voice->setText(QString::number(value));
        });

    //���ƽ���
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

    //��ʼ��Ĭ�ϱ���
    backgroundtodefault();

    //����Ƥ���˵���
    QAction* actiontoDefault = new QAction();
    actiontoDefault->setText("Ĭ�ϱ���");
    actiontoDefault->setIcon(QIcon(":/image/default.png"));

    QAction* actiontoSetting = new QAction();
    actiontoSetting->setText("�Զ��屳��");
    actiontoSetting->setIcon(QIcon(":/image/setting.png"));

    connect(actiontoDefault, &QAction::triggered, this, &MainWidght::backgroundtodefault);
    connect(actiontoSetting, &QAction::triggered, this, &MainWidght::backgroundtosetting);

    menuchangeSkin = new QMenu(this);
    menuchangeSkin->addAction(actiontoDefault);
    menuchangeSkin->addAction(actiontoSetting);

    //��������Ƥ������
    //������ֲ˵���
    connect(ui->btn_skin, &QPushButton::clicked, this, [=]() {
        menuchangeSkin->exec(QCursor::pos());
        });

    painter = new QPainter(this);

    //����
    connect(ui->btn_about, &QPushButton::clicked, this, [=]() {
        QMessageBox::about(this, "About", "MP3���ֲ�������������\n\n"
            "�� �ߣ�����С�Ľ���");
        });

    //ϵͳ���̳�ʼ��
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


//��ȡ��������
void MainWidght::netReply(QNetworkReply* reply)
{
    //��ȡ��Ӧ��Ϣ 200Ϊ����
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << status_code;
    //�ض���
    reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    //���û�д��󷵻�
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        emit finish(data);
    }
    else {
        qDebug() << reply->errorString();
    }
    reply->deleteLater();

}

//��ʾ���
void MainWidght::lyricTextShow(QString lycris)
{
    ui->textBrowser->setFont(QFont("����", 10, QFont::Bold));
    ui->textBrowser->setTextColor(Qt::white);
    ui->textBrowser->setText(lycris);
}

void MainWidght::Search_clicked()
{
    //��ԭ�и����������
    ui->lw_search->clear();
    //�������ݿ����Լ��洢�� ����
    QSqlQuery query;
    QString sql = "delete from searchlist;";
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("Delete searchlist Error."), db.lastError().text());
    }

    //�����û�������������ƣ������������
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

    //������ҳ�ظ������ݣ��������õ�������url_id������ֵ��ŵ����ݿ�
    url_idJsonAnalysis(JsonData);
}

//��������//�����б�
void MainWidght::playSearchMusic()
{
    //˫��������ö�Ӧ�����������ݿ⵱�����ݱ��id��
    int row = ui->lw_search->currentRow();
    qDebug() << "row--" << row;

    //��ѯ�������ݿ��е����ݱ��д洢�����ֵ�������Ϣ
    QSqlQuery query;
    QString sql = QString("select *from searchlist where id=%1;").arg(row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select searchlist table Error."), db.lastError().text());
    }

    //��ѡ�е����ֵ�������Ϣ������ʷ���ݱ���
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
        //��ѡ�е����ֵ�������Ϣ������ʷ���ݱ���
        if (query.next() == NULL) {
            sql = QString("insert into historysong values(%1,'%2','%3','%4')").arg(ui->lw_history->count() + 1).arg(songname).arg(singername).arg(url_id);
            if (!query.exec(sql)) {
                QMessageBox::critical(0, QObject::tr("insert historysong Error."), db.lastError().text());
            }

            //���������������Ʊ��浽lw_history����ʾ
            QString show = songname + "����" + singername;
            QListWidgetItem* item = new QListWidgetItem(show);
            ui->lw_history->insertItem(0, item);

        }
    }
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);
}

//��������//��ʷ�б�
void MainWidght::playHistoryhMusic()
{
    //˫��������ö�Ӧ�����������ݿ⵱�����ݱ��id��
    int row = ui->lw_history->currentRow();
    qDebug() << "row--" << row;
    //��ѯ�������ݿ��е���ʷ��¼�д洢������������Ϣ
    QSqlQuery query;
    QString sql = QString("select *from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select historysong table Error."), db.lastError().text());
    }

    //��¼�ø�������Ϣ
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

    //����
    downloadPlayer(url_id);
    lyricJsonAnalysis(url_id);

    //ɾ���������������ݿ��е�λ���Լ�lw_history�е�λ��
    sql = QString("delete from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("delete historysong table Error."), db.lastError().text());
    }
    sql = QString("update historysong set id=id-1 where id>=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("updata historysong table Error."), db.lastError().text());
    }

    //ɾ����lw_history�е���
    delete ui->lw_history->takeItem(row);

    sql = QString("insert into historysong values(%1,'%2','%3','%4')").arg(ui->lw_history->count() + 1).arg(songname).arg(singername).arg(url_id);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("insert historysong Error."), db.lastError().text());
    }

    //���������������Ʊ��浽lw_history����ʾ
    QString show = songname + "����" + singername;
    QListWidgetItem* item = new QListWidgetItem(show);
    ui->lw_history->insertItem(0, item);

    this->row = 0;

}

//��һ��
void MainWidght::lastplay()
{
    row++;
    if (row > ui->lw_history->count() - 1) {
        row = 0;
    }

    //��ѯ���ݿ���ʷ��¼���д洢��������Ϣ
    QSqlQuery query;
    QString sql = QString("select *from historysong where id=%1;").arg(ui->lw_history->count() - row);
    if (!query.exec(sql)) {
        QMessageBox::critical(0, QObject::tr("select historysong table Error."), db.lastError().text());
    }

    //��ѡ�е����ֵ�������Ϣ������ʷ���ݱ���
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
//��һ��
void MainWidght::nextplay()
{
    //��鲥�ŷ�ʽ
    switch (currentState)
    {
    case 0://ѭ������
        row--;
        if (row < 0) {
            row = ui->lw_history->count() - 1;
        }
        break;
    case 1://�������
        row = QRandomGenerator::global()->bounded(ui->lw_history->count());
        break;
    case 2://����ѭ��
        break;
    default:
        break;
    }

    //��ѯ���ݿ���ʷ��¼���д洢��������Ϣ
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

//����Ĭ�ϱ���
void MainWidght::backgroundtodefault()
{
    //��ȡwidget��palette
    QPalette palette = this->palette();
    if (pixnum > 4)pixnum = 1;
    palette.setBrush(QPalette::Window,
        QBrush(QPixmap(QString(":/image/qwer%1.png").arg(pixnum++)).scaled(//ά�屳��ͼ
            this->size(), Qt::IgnoreAspectRatio,//������ԭͼƬ�ĳ������
            Qt::SmoothTransformation)));//ʹ��ƽ�������ŷ�ʽ
    this->setPalette(palette);

    //if (pixnum > 4)pixnum = 1;
    //painter->drawPixmap(0, 0, this->width(), this->height(), QPixmap(QString(":/image/qwer%1.png").arg(pixnum++)));
}
//�û��Զ��屳��
void MainWidght::backgroundtosetting()
{
    //ѡ���ͼƬ��Ϊ����Ƥ��
    QString strFileName = QFileDialog::getOpenFileName(this, "��ѡ���Զ��屳��ͼƬ",
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(), u8"ͼƬ(*.jpg *.png)");

    //��ȡwidget��palette
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window,
        QBrush(QPixmap(strFileName).scaled(//ά�屳��ͼ
            this->size(), Qt::IgnoreAspectRatio,//������ԭͼƬ�ĳ������
            Qt::SmoothTransformation)));//ʹ��ƽ�������ŷ�ʽ
    this->setPalette(palette);
}


//���ز�����
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

    //������Ҫ���ŵ�����
    player->setMedia(QUrl(mp3url));

    //��������
    player->setVolume(ui->Hslider_voice->value());
    ui->label_voice->setText(QString::number(ui->Hslider_voice->value()));

    ////��������������
    //ui->Hslider_voice->setValue(100);

    //��������
    player->play(); 
    ui->btn_play->setIcon(QIcon(":/image/play1.png"));
    
}

//����http��ҳ
void MainWidght::httpAccess(QString url)
{
    //ʵ�������������������
    request = new QNetworkRequest;
    //��url��ҳ��ַ����request��
    request->setUrl(url);
    //ʵ�����������

    //��������ͷ�����¼
    request->setRawHeader("Cookie", "NMTID=00OtdYjju0yepeluUtFq2gWtCBaToEAAAGRBMz92g");
    request->setRawHeader("User-Agent", "PostmanRuntime/7.40.0");

    manager = new QNetworkAccessManager;
    //ͨ��get�������ϴ���������
    manager->get(*request);

    //����ҳ�ظ���Ϣʱ������finished�źţ�
    connect(manager, &QNetworkAccessManager::finished, this, &MainWidght::netReply);


}

//���ֵ�url_id�����Լ������б���ʾ
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

                    //���������������ƴ���lw_search������ʾ
                    QString show = songname + "����" + singername;
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

//��ʽ���
void MainWidght::lyricJsonAnalysis(QString url_id)
{
    //�����û�������������ƣ������������
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

    //������ҳ�ظ��ĸ��
    QJsonDocument doc = QJsonDocument::fromJson(JsonData);
    if (doc.isObject()) {
        QJsonObject data = doc.object();
        if (data.contains("lyric")) {
            emit lyricShow(data.value("lyric").toString());
            
        }
        else {
            emit lyricShow("���޸��");
        }
        if (doc.isArray()) {
            qDebug() << "Array";
        }
    }
}

//��Ӧϵͳ���̵�˫������
void MainWidght::SystiActicated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        //��ʾ���ؽ���
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
//ϵͳ���̳�ʼ��
void MainWidght::initSysti()
{
    mySystemTray = new QSystemTrayIcon(this);
    mySystemTray->setIcon(QIcon(":/image/qwer.png"));
    connect(mySystemTray, &QSystemTrayIcon::activated, this, &MainWidght::SystiActicated);

    QAction* actionSysquit = new QAction(QIcon(":/image/quit.png"),u8"�˳�����");
    connect(actionSysquit, &QAction::triggered, this, &MainWidght::quitmusicplayer);

    QMenu* pcontextmenu = new QMenu(this);
    pcontextmenu->addAction(actionSysquit);
    mySystemTray->setContextMenu(pcontextmenu);
    mySystemTray->show();

}
//�����˳�����
void MainWidght::quitmusicplayer()
{
    QCoreApplication::quit();

}
