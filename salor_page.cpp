#include "salor_page.h"
#include <QDebug>
#include <QMessageBox>
#include "salor_plugin_factory.h"
#include <QTimer>
#include <QtWebKit>
#include <QtGui>
#include <QTimer>
#include <QByteArray>
#include <downloader.h>

#include "salorprocess.h"

SalorPage::SalorPage(QObject* parent):QWebPage(parent)
{
    //SalorPluginFactory* wpf = new SalorPluginFactory(this);

    //this->setPluginFactory(wpf);
    this->js_error_count = 0;

    qDebug() << "Setup SalorPage signals";
}
void SalorPage::downloadFile(QNetworkRequest request) {
    qDebug() << "Other download called" << request.url().toString();
    QString default_file_name = QFileInfo(request.url().toString()).fileName();
    QString file_name = QFileDialog::getSaveFileName((QWidget*)this,tr("Save File"),default_file_name);
    if (file_name.isEmpty())
        return;
    QNetworkRequest new_request = request;
    new_request.setAttribute(QNetworkRequest::User,file_name);
    QNetworkAccessManager * mng = this->networkAccessManager();
    QNetworkReply *reply = mng->get(new_request);

}
void SalorPage::downloadFile(QNetworkReply *reply) {

    qDebug() << "Download called" << reply->url().toString();
    QString default_file_name = QFileInfo(reply->url().toString()).fileName();
    QString file_name = QFileDialog::getSaveFileName(0,tr("Save File"),default_file_name);
    if (file_name.isEmpty())
        return;
    Downloader * d = new Downloader;
    d->file_name = file_name;
    connect(d,SIGNAL(fileProgressUpdated(int)),this->main,SLOT(setProgress(int)));
    connect(d,SIGNAL(finished()),d,SLOT(deleteLater()));
    connect(d,SIGNAL(addWidget(QWidget*)),this,SLOT(bubbleAddWidget(QWidget*)));
    connect(d,SIGNAL(removeWidget(QWidget*)),this,SLOT(bubbleRemoveWidget(QWidget*)));
    d->setReply(reply);
    //connect(reply,SIGNAL(downloadProgress(qint64,qint64)),d,SLOT(updateFileProgress(qint64,qint64)));

    d->main = this->main;
}
void SalorPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID  ) {
    //QMessageBox::critical(0, QObject::tr("Critical Script Error"), QString() + "A Javascript error Occurred: '" + message + "'\nat line " + QString::number(lineNumber) + "\nin " + sourceID);
    if (this->js_error_count >= 1000) {
        return;
    }
    QString err = QString() + "A Javascript error Occurred: " + message + "'\nat line " + QString::number(lineNumber) + "\nin " + sourceID;

    this->js_error_count++;
    if (this->js_error_count > 30) {
        this->js_error_count = 1000;
        //this->setAttribute(QWebSettings::JavascriptEnabled,"off");
        //QMessageBox::critical(0, QObject::tr("Critical Script Error"), "Scripting is being stopped, there were more than 100 errors. please call tech support NOW to get this resolved. The errors have been logged.");
        return;
    }
    if (this->js_error_count == 10) {
        //QMessageBox::critical(0, QObject::tr("Critical Script Error"), "There have been too many errors, please call tech support now to get this resolved. The errors have been logged.");
    } else {
       // emit generalSnap(err);
    }

    qDebug() <<  "xxxA Javascript error Occurred: " << err;
}
QString SalorPage::chooseFile(QWebFrame* /*frame*/, const QString& /*suggestedFile*/) {
  QString file_name = QFileDialog::getOpenFileName();
  if (!file_name.isEmpty()) {
    return file_name;
  }
  qDebug() << "Returing null from SalorPage::chooseFile";
  return QString::null;
}

bool SalorPage::javaScriptConfirm(QWebFrame* /*frame*/, const QString& /*msg*/) {
  return true;
}

bool SalorPage::javaScriptPrompt(QWebFrame* /*frame*/,
                           const QString& /*msg*/,
                           const QString& /*defaultValue*/,
                           QString* /*result*/) {
  return true;
}


void SalorPage::javaScriptAlert(QWebFrame* /*frame*/, const QString& msg) {
   qDebug() << "[alert]" << msg;

  if (mAlertString == msg) {
    QTimer::singleShot(10, mSalorCapture, SLOT(Delayed()));
  }
}

QString SalorPage::userAgentForUrl(const QUrl& url) const {

  if (!mUserAgent.isNull())
    return mUserAgent;

  return QWebPage::userAgentForUrl(url);
}

void SalorPage::setUserAgent(const QString& userAgent) {
  mUserAgent = userAgent;
}

void SalorPage::setAlertString(const QString& alertString) {
  mAlertString = alertString;
}

QString SalorPage::getAlertString() {
  return mAlertString;
}

void SalorPage::setSalorCapture(SalorCapture* SalorCapture) {
  mSalorCapture = SalorCapture;
}

void SalorPage::setPrintAlerts(bool printAlerts) {
  mPrintAlerts = printAlerts;
}

void SalorPage::setAttribute(QWebSettings::WebAttribute option,
                       const QString& value) {

  if (value == "on")
    settings()->setAttribute(option, true);
  else if (value == "off")
    settings()->setAttribute(option, false);
  else
    (void)0; // TODO: ...
}

/* Salor Capt Definitions */

SalorCapture::SalorCapture(SalorPage* page, const QString& output, int delay,
                   const QString& scriptProp, const QString& scriptCode) {
  mPage = page;
  mOutput = output;
  mDelay = delay;
  mSawInitialLayout = false;
  mSawDocumentComplete = false;
  mScriptProp = scriptProp;
  mScriptCode = scriptCode;
  mScriptObj = new QObject();

  qDebug() << "In SalorCapture";
  mPage->setSalorCapture(this);

  qDebug() << "setSalorCapture done";
}

void SalorCapture::InitialLayoutCompleted() {
    qDebug() << "in InitialLayoutCompleted";
  mSawInitialLayout = true;

  if (mSawInitialLayout && mSawDocumentComplete)
    TryDelayedRender();
}

void SalorCapture::DocumentComplete(bool /*ok*/) {
  qDebug() << "In DocumentComplete";

  saveSnapshot();
  return;
  mSawDocumentComplete = true;

  if (mSawInitialLayout && mSawDocumentComplete)
    TryDelayedRender();
}

void SalorCapture::JavaScriptWindowObjectCleared() {

}

void SalorCapture::TryDelayedRender() {
    qDebug() << "TryDelayRender called";
  if (!mPage->getAlertString().isEmpty())
    return;

  if (mDelay > 0) {
    QTimer::singleShot(mDelay, this, SLOT(Delayed()));
    return;
  }

  saveSnapshot();
}

void SalorCapture::Timeout() {
  saveSnapshot();
}

void SalorCapture::Delayed() {
  saveSnapshot();
}
void SalorCapture::saveSnapshot() {
    qDebug() << "saveSnapshot was called";
    QWebFrame *mainFrame = mPage->mainFrame();

    QSize size(800,480);
    //mainFrame->contentsSize(size);

    mPage->setViewportSize(size); //mainFrame->contentsSize()
    QImage image(size, QImage::Format_ARGB32); // mPage->viewportSize()

    QPainter painter;
    painter.begin(&image);
    mainFrame->render(&painter);
    painter.end();

    // Here is where we hook in.
    qDebug() << "Saving to: " << mOutput;
    image.save(mOutput, "bmp");
    SalorProcess *sp = new SalorProcess(this);
    qDebug() << "Running poledancer";
    sp->run("poledancer",QStringList() << "-dlo" <<  mOutput,2000);

    delete sp;
    emit done();
    //display_link_write_image(mOutput.toAscii());
}
void SalorCapture::DocumentPrint() {
    qDebug() << "DocumentPrint was called";
    QWebFrame *mainFrame = mPage->mainFrame();

    QSize size(800,480);
    //mainFrame->contentsSize(size);

    mPage->setViewportSize(size); //mainFrame->contentsSize()
    QPrinter printer;
    printer.setPageSize(QPrinter::A4);
    printer.setPageMargins(10, 10, 10, 10, QPrinter::Millimeter);
    printer.setColorMode(QPrinter::Color);
    QPrintDialog* dialog = new QPrintDialog(&printer, 0);
    if (dialog->exec() == QDialog::Accepted)
    {
         mainFrame->print(&printer);
    }
    emit done();
}
