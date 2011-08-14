#include "mainwindow.h"
#include <QWebPage>
#include <QDesktopServices>
#include <QSysInfo>
#include "salor_page.h"
#include <QApplication>
#include <QKeyEvent>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{

}

void MainWindow::init() {
    this->shown = false;
    this->scs = new SalorCustomerScreen(this);
    SalorPage* page = new SalorPage(this);
    webView = new QWebView();
    webView->setPage((QWebPage*)page);
/*
    if (s->getValue("PluginsEnabled").toBool() == true) {
        defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);
    }
    if (s->getValue("OfflineStorageEnabled").toBool() == true) {
        defaultSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled,true);
        defaultSettings->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    }
*/

    setCentralWidget(webView);
    webView->show();
    webView->load(this->to_url);
    connectSlots();
}
QWebView* MainWindow::getWebView() {
    return this->webView;
}

void MainWindow::connectSlots() {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(repaintViews()));
    timer->start(500);
    connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this,SLOT(addJavascriptObjects()));
    connect(webView->page(), SIGNAL(windowCloseRequested()), this,SLOT(windowCloseRequested()));
    this->installEventFilter(this);
}
void MainWindow::addJavascriptObjects() {
    if (!this->shown) {
       this->shown = true; // i.e. this is ready
    }
    attach();
}
void MainWindow::linkClicked(QUrl url) {


}
void MainWindow::repaintViews() {

    webView->update();
}
void MainWindow::attach(){
    this->webView->page()->mainFrame()->addToJavaScriptWindowObject("CustomerScreen", this->scs);
    this->webView->page()->mainFrame()->addToJavaScriptWindowObject("Salor", this);
}



void MainWindow::changeEvent(QEvent *e){
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        //xlation code
        break;
    default:
        break;
    }
}
void MainWindow::shutdown() {
  windowCloseRequested();
}
void MainWindow::windowCloseRequested() {
  qDebug() << "Called";
  QApplication::closeAllWindows();
}
bool MainWindow::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
      case QEvent::KeyRelease: 
          {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
            QString key = "salorKeyRelease(";
            key = key + QString::number(keyEvent->key());
            key = key +  "); null";
            this->webView->page()->mainFrame()->evaluateJavaScript(key);
          }
          break;
      default:
        return false;
          break;
    }
    return false;
}
