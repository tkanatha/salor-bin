#ifndef SALORJSAPI_H
#define SALORJSAPI_H

#include <QObject>
#include "common_includes.h"
#include "drawerobserver.h"
#include "salorprocess.h"

class SalorJsApi : public QObject
{
    Q_OBJECT
public:
    explicit SalorJsApi(QObject *parent = 0);
    QWebView *webView;
    DrawerObserverThread * drawer_thread;

signals:
    void cuteDataRead(QString);
    void _cuteWriteData(QString);
    
public slots:
    void poleDancer(QString path, QString message);
    void newOpenCashDrawer(QString addy);
    void startDrawerObserver(QString addy);
    void stopDrawerObserver();
    void _cashDrawerClosed();
    void printPage();
    void playSound(QString name);
    void echo(QString msg);
    void mimoRefresh(QString path,int h, int w);
    void mimoImage(QString imagepath);
    QStringList ls(QString path,QStringList filters);
    void shutdown();
    void cuteWriteData(QString);
    void _cuteBubbleDataRead(QString data);
    QString weigh(QString addy, int protocol);
    
};

#endif // SALORJSAPI_H
