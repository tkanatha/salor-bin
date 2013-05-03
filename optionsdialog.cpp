#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "common_includes.h"
#include <QPrinterInfo>
#include <QList>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    ui->URLEdit->setText(_get(QString("salor.url")).toString());
    _ready = false;
#ifdef MAC
    int i;
    cups_dest_t *dests, *dest;
    int num_dests = cupsGetDests(&dests);
    for (i = num_dests, dest = dests; i > 0; i--, dest++) {
        //qDebug() << "Adding" << list.at(i).printerName();
        QString name = dest->name;
        ui->printerComboBox->addItem(name,QVariant(name));
     }

#endif
#ifdef LINUX
    QDir dev("/dev");
    dev.setFilter(QDir::AllEntries);
    QFileInfoList list = dev.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        ui->printerComboBox->addItem(fileInfo.fileName(),QVariant(fileInfo.fileName()));
    }
#endif
#ifdef WIN32
    LPBYTE pPrinterEnum;
    DWORD pcbNeeded, pcbReturned;
    PRINTER_INFO_2 *piTwo = NULL;
    EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,NULL,0,&pcbNeeded,&pcbReturned);
    pPrinterEnum = new BYTE[pcbNeeded];
    if (!EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,pPrinterEnum,pcbNeeded,&pcbNeeded,&pcbReturned)) {
        bug(could not enumerate printers);
    } else {
        piTwo = ((PRINTER_INFO_2*)pPrinterEnum);
        for (int i = 0; i < pcbReturned; i++) {
            QString name = QString::fromWCharArray(piTwo[i].pPrinterName);
            ui->printerComboBox->addItem(name,QVariant(name));
        }
    }
#endif
    int index = ui->printerComboBox->findText(_get("salor.thermal_printer").toString());
    //qDebug() << "index is: " << index << "get is:" << _get("salor.thermal_printer");
    if (index != -1) {
      ui->printerComboBox->setCurrentIndex(index);
    }
    connect(ui->printerComboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(on_printerComboBox_currentIndexChanged(QString)));
    _ready = true;
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_printerComboBox_currentIndexChanged(QString name) {
    if(this->_ready == false)
        return;
    _set("salor.thermal_printer",name);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(name);

}

void OptionsDialog::on_ClearCacheButton_clicked() {
    emit clearCache();
}
