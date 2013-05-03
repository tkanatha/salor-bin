#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "salorpage.h"
#include "network.h"
#include "salorprinter.h"
#include "optionsdialog.h"
#include "salorcookiejar.h"
#include "downloader.h"
#include "common_includes.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    this->shown = false;
    progress = 0;
    QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    QWebSettings::globalSettings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PrintElementBackgrounds, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    webView = new QWebView();
    this->page = new SalorPage(this);
    this->page->main = this;
    this->page->setForwardUnsupportedContent(true);
    connect(this->page,SIGNAL(unsupportedContent(QNetworkReply*)),this->page,SLOT(downloadFile(QNetworkReply*)));
    connect(this->page,SIGNAL(addWidget(QWidget*)),this,SLOT(addStatusBarWidget(QWidget*)));
    connect(this->page,SIGNAL(removeWidget(QWidget*)),this,SLOT(removeStatusBarWidget(QWidget*)));
    webView->setPage(this->page);
    connect(this->page,SIGNAL(fileProgressUpdated(int)),this,SLOT(setProgress(int)));
    Network* net = new Network(this);
    webView->page()->setNetworkAccessManager(net);
    SalorCookieJar * jar = new SalorCookieJar(this);

    webView->page()->networkAccessManager()->setCookieJar(jar);
    this->sp = new SalorPrinter(this);
    this->js = new SalorJsApi(this);
    this->js->webView = this->webView;
    //connect(page,SIGNAL(generalSnap(QString)),this->js,SLOT(generalSnap(QString)));
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(PathCache);
    qDebug() << PathCache << " is the cache location";
    webView->page()->networkAccessManager()->setCache(diskCache);
    setCentralWidget(webView);

    webView->show();
    statusBar = new QStatusBar(this);
    this->setStatusBar(statusBar);
    status_bar_progressBar = new QProgressBar();
    status_bar_progressBar->setMinimum(0);
    status_bar_progressBar->setMaximum(100);
    status_bar_urlLabel = new QLabel( "Location" );
    this->statusBar->addPermanentWidget(status_bar_urlLabel);
    this->statusBar->addPermanentWidget(status_bar_progressBar);

    webView->load(this->to_url);
    connectSlots();
}

void MainWindow::connectSlots() {
    connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this,SLOT(addJavascriptObjects()));
    connect(webView->page(), SIGNAL(windowCloseRequested()), this,SLOT(windowCloseRequested()));
    connect(this->page->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),this->page,SLOT(resetJsErrors()));
    connect(webView, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(webView, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P), this);
    connect( shortcut, SIGNAL(activated()), this->js, SLOT(printPage()));

    QShortcut *f2 = new QShortcut(QKeySequence(Qt::Key_F2), this);
    connect( f2, SIGNAL(activated()), this, SLOT(lastFiveOrders()));

    QShortcut *ctrlf2 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F2), this);
    connect( ctrlf2, SIGNAL(activated()), this, SLOT(customersIndex()));

    QShortcut *f3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
    connect( f3, SIGNAL(activated()), this, SLOT(showSearch()));

    QShortcut *f4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect( f4, SIGNAL(activated()), this, SLOT(showCashDrop()));

    QShortcut *f5 = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect( f5, SIGNAL(activated()), this, SLOT(endDayReport()));

    QShortcut *ctrlins = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Insert), this);
    connect( ctrlins, SIGNAL(activated()), this, SLOT(editLastAddedItem()));

    QShortcut *endk = new QShortcut(QKeySequence(Qt::Key_End), this);
    connect( endk, SIGNAL(activated()), this, SLOT(completeOrder()));

    QShortcut *zoomin = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this);
    connect( zoomin, SIGNAL(activated()), this, SLOT(incZoom()));

    QShortcut *zoomout = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this);
    connect( zoomout, SIGNAL(activated()), this, SLOT(decZoom()));

    #ifdef MAC
        QShortcut *showOptions = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_O), this);
        connect( showOptions, SIGNAL(activated()), this, SLOT(showOptionsDialog()));
    #else
        QShortcut *showOptions = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this);
        connect( showOptions, SIGNAL(activated()), this, SLOT(showOptionsDialog()));
    #endif

    QSettings settings("JolieRouge", "Salor");
    qreal z = settings.value("zoomFactor").toReal();
    if (z) {
      this->webView->page()->mainFrame()->setZoomFactor(z);
    }
}

QWebView* MainWindow::getWebView() {
    return this->webView;
}

void MainWindow::incZoom() {
  QSettings settings("JolieRouge", "Salor");
  qreal z = this->webView->page()->mainFrame()->zoomFactor();
  z = z + 0.05;
  this->webView->page()->mainFrame()->setZoomFactor(z);
  settings.setValue("zoomFactor",z);
}

void MainWindow::decZoom() {
  QSettings settings("JolieRouge", "Salor");
  qreal z = this->webView->page()->mainFrame()->zoomFactor();
  z = z - 0.05;
  this->webView->page()->mainFrame()->setZoomFactor(z);
  settings.setValue("zoomFactor",z);
}

void MainWindow::addStatusBarWidget(QWidget *w) {
    qDebug() << "Adding widget";
    this->statusBar->addWidget(w);
}

void MainWindow::removeStatusBarWidget(QWidget *w) {
    this->statusBar->removeWidget(w);
}

void MainWindow::setProgress(int p) {
    qDebug() << "set progress called" << QString::number(p);
    progress = p;
    adjustTitle();
}

void MainWindow::finishLoading(bool) {
     progress = 100;
     adjustTitle();
}

void MainWindow::showOptionsDialog() {
    OptionsDialog* d = new OptionsDialog(this);
    connect(d,SIGNAL(navigateToUrl(QString)),this,SLOT(navigateToUrl(QString)));
    connect(d,SIGNAL(clearCache()),webView->page()->networkAccessManager()->cache(),SLOT(clear()));
    d->show();
}

void MainWindow::executeJS(QString &js) {
    webView->page()->mainFrame()->evaluateJavaScript(js);
}

void MainWindow::adjustTitle() {
     if (progress <= 0 || progress >= 100)
         setWindowTitle(webView->title());
     else
         setWindowTitle(QString("%1 (%2%)").arg(webView->title()).arg(progress));
     this->status_bar_progressBar->setValue(progress);
     this->status_bar_urlLabel->setText(this->webView->url().encodedHost());
}

void MainWindow::navigateToUrl(QString url) {
    this->webView->load(QUrl(url));
}

void MainWindow::customersIndex() {
    this->webView->page()->mainFrame()->evaluateJavaScript("window.location = '/customers';");
}

void MainWindow::endDayReport() {
    this->webView->page()->mainFrame()->evaluateJavaScript("window.location = '/orders/report_day';");
}

void MainWindow::editLastAddedItem() {
    this->webView->page()->mainFrame()->evaluateJavaScript("editLastAddedItem();");
}

void MainWindow::lastFiveOrders() {
    this->webView->page()->mainFrame()->evaluateJavaScript("onF2Key();");
}

void MainWindow::completeOrder() {
    this->webView->page()->mainFrame()->evaluateJavaScript("onEndKey();");
}

void MainWindow::showSearch() {
    this->webView->page()->mainFrame()->evaluateJavaScript("showSearch();");
}

void MainWindow::showCashDrop() {
    this->webView->page()->mainFrame()->evaluateJavaScript("show_cash_drop();");
}

void MainWindow::addJavascriptObjects() {
    if (!this->shown) {
       this->shown = true; // i.e. this is ready
    }
    attach();
}

void MainWindow::repaintViews() {
    webView->update();
}

void MainWindow::attach(){
    this->webView->page()->mainFrame()->addToJavaScriptWindowObject("SalorPrinter", this->sp);
    this->webView->page()->mainFrame()->addToJavaScriptWindowObject("Salor", this->js);
    this->page->resetJsErrors();
}

void MainWindow::windowCloseRequested() {
  QApplication::closeAllWindows();
}

