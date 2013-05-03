#ifndef SALORCOOKIEJAR_H
#define SALORCOOKIEJAR_H

#include "common_includes.h"

class SalorCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit SalorCookieJar(QObject *parent = 0);
    virtual ~SalorCookieJar();
    virtual bool setCookiesFromUrl(const QList<QNetworkCookie>&, const QUrl&);
    void setDiskStorageEnabled(bool);

private:
    void extractRawCookies();
    QList<QByteArray> m_rawCookies;
    bool m_storageEnabled;
    QFile m_file;
    QTimer m_timer;
    
signals:
    
public slots:
    void scheduleSaveToDisk();
    void loadFromDisk();
    void reset();

private slots:
    void saveToDisk();

    
};

#endif // SALORCOOKIEJAR_H
