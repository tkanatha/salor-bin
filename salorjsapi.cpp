#include "salorjsapi.h"
#include "common_includes.h"
#include "drawerobserver.h"
#include "customerscreen.h"
#include "salorprinter.h"
#include "scale.h"

SalorJsApi::SalorJsApi(QObject *parent, QNetworkAccessManager *nm) :
    QObject(parent)
{
    networkManager = nm;
    drawerObserver = new DrawerObserver();
    drawerThread = new QThread(this);
    connect(drawerThread, SIGNAL(started()),
            drawerObserver, SLOT(start()));
    connect(drawerThread, SIGNAL(finished()),
            this, SLOT(drawerThreadFinished()));
    drawerObserver->moveToThread(drawerThread);
}

void SalorJsApi::echo(QString msg) {
    QDateTime  d = QDateTime::currentDateTime();
    qDebug() << "[echo at " << d << "] " << msg;
}

void SalorJsApi::playSound(QString name) {
#ifdef LINUX
    qDebug() << "[SalorJsApi]" << "[playSound] Beginning.";
    SalorProcess *sp = new SalorProcess(this);
    sp->run("aplay", QStringList() << "/usr/share/salor-bin/sounds/" + name + ".wav",3000);
    delete sp;
#endif
#ifdef WIN32
    QSound::play("sounds/" + name + ".wav");
#endif
    qDebug() << "[SalorJsApi]" << "[playSound] Ending.";
}

/* shows printer dialog */
void SalorJsApi::printPage() {
    qDebug() << "SalorJsApi::printPage";
    QPrinter printer;
    printer.setPageSize(QPrinter::A4);
    printer.setPageMargins(10, 10, 10, 10, QPrinter::Millimeter);
    printer.setColorMode(QPrinter::Color);
    QPrintDialog* dialog = new QPrintDialog(&printer, 0);
    if (dialog->exec() == QDialog::Accepted) {
        webView->page()->mainFrame()->print(&printer);
        if (printer.outputFileName().indexOf(".pdf") != -1) {
            chmod(printer.outputFileName().toLatin1().data(),0666);
        }
    }
    delete dialog;
    qDebug() << "SalorJsApi::printPage complete";
}

void SalorJsApi::newOpenCashDrawer(QString addy) {
#ifdef LINUX
    QByteArray openCode;
    Serialport *serialport;
    int count;

    openCode = QByteArray("\x1B\x70\x30\x55\x55");

    serialport = new Serialport(addy);
    serialport->open();
    count = serialport->write(openCode);
    serialport->close();

    qDebug() << "[SalorJsApi]" << "[newOpenCashDrawer]" << "Wrote" << count << "bytes to" << addy;
#endif
}

void SalorJsApi::startDrawerObserver(QString path) {
  qDebug() << "[SalorJsApi]" << "[startDrawerObserver] Beginning with argument" << path;
  if (drawerThread->isRunning() == false) {
      drawerObserver->mPath = path;
      drawerThread->start();
      drawerThread->quit(); // this will finish the thread AFTER the while loop of the start method has finished.
  } else {
      qDebug() << "[SalorJsApi]" << "[startDrawerObserver] drawerThread is already running. Doing nothing.";
  }
  qDebug() << "[SalorJsApi]" << "[startDrawerObserver] Ending.";


  /*
  if (drawerObserver) {
      qDebug() << "[SalorJsApi]" << "[startDrawerObserver] A drawerObserver was already instantiated. Doing nothing.";
      return;
  } else {
      drawerObserver = new DrawerObserver();
      connect(drawerObserver, SIGNAL(drawerCloseDetected()), this, SLOT(drawerCloseDetected()));
      drawerObserver->mPath = path;
      drawerObserver->start();
      qDebug() << "[SalorJsApi]" << "[startDrawerObserver] Instantiated a new drawerObserver which is now observing.";
  }
  */
}

void SalorJsApi::stopDrawerObserver() {
    qDebug() << "[SalorJsApi]" << "[stopDrawerObserver]" << "Setting member variable doStop to true";
    drawerObserver->doStop = true;
    /*
    if (drawerObserver) {
        drawerObserver->stop();
        drawerObserver->deleteLater();
        drawerObserver = 0;
        qDebug() << "[SalorJsApi]" << "[stopDrawerObserver]" << "Stopped and deleted it.";
    } else {
        qDebug() << "[SalorJsApi]" << "[stopDrawerObserver]" << "no drawerObserver instantiated. Doing nothing.";
    }
    */
}

void SalorJsApi::drawerThreadFinished() {
    qDebug() << "[SalorJsApi]" << "[drawerThreadFinished]";
    if (drawerObserver->doStop == true) {
        qDebug() << "[SalorJsApi]" << "[drawerThreadFinished]" << "drawerObserver was interrupted manually. Not calling JS";
        drawerObserver->doStop = false;
    }

    if (drawerObserver->drawerClosed == true) {
        qDebug() << "[SalorJsApi]" << "[drawerThreadFinished]" << "evaluating Javascript 'onCashDrawerClose()";
        webView->page()->mainFrame()->evaluateJavaScript("onCashDrawerClose();");
        drawerObserver->drawerClosed = false;
    }
    /*
    drawerObserver->deleteLater();
    drawerObserver = 0;
    webView->page()->mainFrame()->evaluateJavaScript("onCashDrawerClose();");
    */
}

void SalorJsApi::printURL(QString printer, QString url, QString confirm_url) {
    SalorPrinter *salorprinter = new SalorPrinter(this, networkManager, printer);
    salorprinter->printURL(url);
    // printer will delete itself later
    qDebug() << "[SalorJsApi]" << "[printURL] Ending.";
}

void SalorJsApi::printText(QString printer, QString text) {
    SalorPrinter *salorprinter = new SalorPrinter(this, networkManager, printer);
    QByteArray bytes;
    bytes = QByteArray();
    bytes.append(text);
    salorprinter->print(bytes);
    // salorprinter will delete itself later
    qDebug() << "[SalorJsApi]" << "[printText] Ending.";
}

QStringList SalorJsApi::ls(QString path,QStringList filters) {
    QDir d(path);
    if (d.exists()) {
        d.setFilter(QDir::Files | QDir::Hidden | QDir::System);
        return d.entryList(filters);
    }
    return QStringList();
}

/* output plain text to Serial pole display */
void SalorJsApi::poleDancer(QString path, QString message) {
    SalorProcess *sp = new SalorProcess(this);
    qDebug() << "calling poledancer " << path << " " << message;
    sp->run("poledancer",QStringList() << "-p" << path << message, 2000);
    delete sp;
    qDebug() << "[SalorJsApi]" << "[poleDancer] Ending.";
}

/* output screenshot of url */
void SalorJsApi::mimoRefresh(QString path,int h, int w) {
    qDebug() << "calling mimoRefresh";
    CustomerScreen *cs = new CustomerScreen(this);
    cs->refresh(path, h, w);
    qDebug() << "[SalorJsApi]" << "[mimoRefresh] Ending.";
}

/* output image file */
void SalorJsApi::mimoImage(QString imagepath) {
    qDebug() << "Calling mimoImage";
    SalorProcess *sp = new SalorProcess(this);
    sp->run("poledancer",QStringList() << "-dlo" <<  imagepath, 3000);
    delete sp;
    qDebug() << "[SalorJsApi]" << "[mimoImage] Ending.";
}

QString SalorJsApi::weigh(QString addy, int protocol) {
  QString weight;
  Scale *sc = new Scale(addy, protocol);
  weight = sc->read();
  return weight;
}

void SalorJsApi::cuteWriteData(QString data) {
    qDebug() << "Writing Data";
    emit _cuteWriteData(data);
}

void SalorJsApi::_cuteBubbleDataRead(QString data) {
    emit cuteDataRead(data);
}

void SalorJsApi::shutdown() {
    int pid = getpid();
    qDebug() << "Shutdown called.";
    QApplication::closeAllWindows();
#ifdef LINUX
    qDebug() << "[SalorJsApi]" << "[shutdown] Calling kill for pid.";
    kill(pid,SIGKILL); // to make really sure
#endif
}
