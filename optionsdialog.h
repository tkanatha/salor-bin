#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "salorprinter.h"
//#include "mainwindow.h"
#include <common_includes.h>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OptionsDialog(QWidget *parent = 0, QNetworkAccessManager *nm = 0);
    ~OptionsDialog();
    QString customerScreenId;
    void init();
    //MainWindow *main;
    
private:
    Ui::OptionsDialog *ui;
    bool auth_tried;
    QNetworkAccessManager *networkManager;
    QSignalMapper *signalMapper;
    QMap<QString, QComboBox *> localPrinterInputWidgetMap;
    void setupPrinterCombos();
    SalorPrinter *sp;

signals:
    void navigateToUrl(QString);
    void clearCache();
    void sendJS(QString &js);
    void printTimerStart();
    void setPrinterCounter(int value);
    void setPrinterNames();

public slots:
    void on_goButton_clicked();
    void on_clearCacheButton_clicked();

private slots:
    void on_urlEditInput_textChanged(QString value);
    void on_updateSettingsButton_clicked();
    void on_printUrlInput_textChanged(const QString &arg1);
    void on_printUsernameInput_textChanged(const QString &arg1);
    void localPrinterInputWidgetChanged(QString text);
    void on_printNowButton_clicked();
    void on_printTestButton_clicked();
    void on_kioskCheckBox_clicked(bool checked);
    void onError(QNetworkReply::NetworkError error);
    void onPrintInfoFetched();
    void on_pushButton_clicked();
};

#endif // OPTIONSDIALOG_H
