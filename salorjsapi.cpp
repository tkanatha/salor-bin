#include "salorjsapi.h"
SalorJSApi::SalorJSApi(QObject *parent) :
    QObject(parent)
{
    //this->credit_thread = new CuteCredit(0);
    //connect(this->credit_thread,SIGNAL(dataRead(QString)),this,SLOT(_cuteBubbleDataRead(QString)));
   // connect(this,SIGNAL(_cuteWriteData(QString)),this->credit_thread,SLOT(writeData(QString)));
    //this->credit_thread->start();
}
void SalorJSApi::playSound(QString name) {
    SalorProcess *sp = new SalorProcess(this);
    sp->run("aplay", QStringList() << "/usr/share/sounds/salor/" + name + ".wav");

}
void SalorJSApi::printPage() {
    QPrinter printer;
    printer.setPageSize(QPrinter::A4);
    printer.setPageMargins(10, 10, 10, 10, QPrinter::Millimeter);
    printer.setColorMode(QPrinter::Color);
        QPrintDialog* dialog = new QPrintDialog(&printer, 0);
        if (dialog->exec() == QDialog::Accepted)
        {
         this->webView->page()->mainFrame()->print(&printer);
         if (printer.outputFileName().indexOf(".pdf") != -1) {
            chmod(printer.outputFileName().toLatin1().data(),0666);
         }
        }
}
QStringList SalorJSApi::ls(QString path,QStringList filters) {
    QDir d(path);
    if (d.exists()) {
        d.setFilter(QDir::Files | QDir::Hidden | QDir::System);
        return d.entryList(filters);
    }
    return QStringList();
}

void SalorJSApi::poleDancer(QString path, QString message) {
    SalorProcess *sp = new SalorProcess(this);
    qDebug() << path << " " << message;
    sp->run("pole-dancer",QStringList() << "-p" << path << message);
}
void SalorJSApi::mimoRefresh(QString path,int h, int w) {
    SalorCustomerScreen *scs = new SalorCustomerScreen(this);
    scs->refresh(path,h,w);
}
void SalorJSApi::completeOrderSnap(QString order_id) {
    QSize size(800,480);
    QImage image(size, QImage::Format_Mono); // mPage->viewportSize()

    QPainter painter;
    painter.begin(&image);
    this->webView->page()->mainFrame()->render(&painter);
    painter.end();
    QString name = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss") + "_" + order_id + ".png";
    // Here is where we hook in.
    //qDebug() << "Saving to: " << mOutput;
    image.save("/opt/salor_pos/logs/images/" + name, "png",1);
}
void SalorJSApi::generalSnap(QString msg) {
    qDebug() << "GeneralSnap called";
    QSize size(800,480);
    QImage image(size, QImage::Format_RGB16); // mPage->viewportSize()

    QPainter painter;
    painter.begin(&image);
    this->webView->page()->mainFrame()->render(&painter);
    painter.end();
    QString name = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss") + QString::number(qrand());
    QDir().mkpath("/opt/salor_pos/logs/images/");
    QFile m("/opt/salor_pos/logs/images/" + name + ".txt");
    if (m.open(QIODevice::ReadWrite)) {
        m.write(msg.toAscii());
        m.write(this->webView->page()->mainFrame()->toHtml().toAscii());
        m.close();
    } else {
        qDebug() << "Could not create file";
    }
    qDebug() << "writing: " << "/opt/salor_pos/logs/images/" + name << "png";
    image.save("/opt/salor_pos/logs/images/" + name + ".png", "png",1);
}
QString SalorJSApi::toperScale(QString addy) {
  int fd, j, count;
  char * weight;
  fd = open_serial_port_for_scale(addy.toLatin1().data());
  request_weight_toperczer_f200_samsung_spain(fd);
  usleep(100000); // sleep 100ms until bytes are in the buffer. 50ms works too.
  weight = read_weight_toperczer_f200_samsung_spain(fd);
  close(fd);
  //qDebug() << "Reading from Toper: " << QString::number(weight);
  //return QString::number(weight);
  return weight;
}

void SalorJSApi::newOpenCashDrawer(QString addy) {
    int fd;
    int count;
    qDebug() << "Attempting to open CashDrawer at " << addy;
    fd = open_serial_port_for_cash_drawer(addy.toLatin1().data());
    if (fd <= 0) {
        qDebug() << "CashDrawer failed to open!";
       // return;
    }
    count = write(fd, "\x1D\x61\xFF", 3);
    qDebug() << "Wrote "  << count << " bytes to enable printer feedback.";
    usleep(5000); //50ms
    count = write(fd, "\x1B\x70\x00\xFF\x01", 5);
    qDebug() << "Wrote "  << count << " bytes to open cash drawer.";
    close(fd);
}

void SalorJSApi::startDrawerObserver(QString addy) {
  DrawerObserverThread * drawerobserver = new DrawerObserverThread(this);
  SalorJSApi::drawer_thread = drawerobserver;
  drawerobserver->addy = addy;
  connect(drawerobserver,SIGNAL(cashDrawerClosed()),this,SLOT(_cashDrawerClosed()));
  connect(drawerobserver,SIGNAL(finished()),drawerobserver,SLOT(deleteLater()));
  drawerobserver->start();
}

void SalorJSApi::stopDrawerObserver() {
  SalorJSApi::drawer_thread->stop_drawer_thread = true;
}

void SalorJSApi::_cashDrawerClosed() {
    qDebug() << "Drawer is closed. Calling JS complete_order_hide();.\n";
    this->webView->page()->mainFrame()->evaluateJavaScript("complete_order_hide();");
}

void SalorJSApi::shutdown() {
  QApplication::closeAllWindows();
}
void SalorJSApi::cuteWriteData(QString data) {
    qDebug() << "Writing Data";
    emit _cuteWriteData(data);
}
void SalorJSApi::_cuteBubbleDataRead(QString data) {
    emit cuteDataRead(data);
}
