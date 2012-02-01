#ifndef SALORJSAPI_H
#define SALORJSAPI_H

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QMap>
#include <QWebView>
#include <QPrinter>
#include <QPrintDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QApplication>
#include <QWebFrame>
#include <QWebPage>
#include <sys/types.h>
#include <sys/stat.h>
#include "salorjsapi.h"
#include "salorprocess.h"
#include "salorprinter.h"
#include "salor_customer_screen.h"
#include "scales.h"
#include "cashdrawer.h"
class SalorJSApi : public QObject
{
    Q_OBJECT
public:
    explicit SalorJSApi(QObject *parent = 0);
    QWebView *webView;
signals:

public slots:
 void poleDancer(QString path, QString message);
 QString toperScale(QString addy);
 QString testScale() {
   qDebug() << "Reading from Test: " << "2.754";
   return QString("2.754");
 }
 void newOpenCashDrawer(QString addy);
 void shutdown();
 void printPage();
 QStringList ls(QString path,QStringList filters);
 void completeOrderSnap(QString order_id);
 void playSound(QString name);
 void mimoRefresh(QString path,int h, int w);
};

#endif // SALORJSAPI_H