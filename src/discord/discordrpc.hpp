#ifndef PEMDAS_DISCORDRPC_HPP
#define PEMDAS_DISCORDRPC_HPP

#ifdef PEMDAS_ENABLE_DISCORDRPC
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QStandardPaths>
#include <QJsonArray>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <discord_rpc.h>
#include <utility>
#include <curl/curl.h>

#include "player.hpp"
#include "utils.hpp"

struct discordapp {
    QString appid;
    QString token;
    bool coverart;
};

struct asset {
    QString key;
    QString id;

    bool operator==(const asset &other) const {
        return this->key == other.key && this->id == other.id;
    }

    bool operator!=(const asset &other) const {
        return !(*this == other);
    }
};

Q_DECLARE_METATYPE(asset)

static size_t curl_write(void *ptr, size_t size, size_t nmemb, QString *data) {
    data->append(static_cast<char *>(ptr));
    return size * nmemb;
}

static bool send_delete_asset(const asset &a, const QString &token, const QString &appid) {
    CURL *curl = curl_easy_init();

    if (!curl) {
        qDebug() << "discordrpc::delete_asset: Failed to init curl";
        return false;
    }

    QString url = "https://discordapp.com/api/oauth2/applications/" + appid + "/assets/" + a.id;
    QString bearer_header = "authorization: " + token;

    curl_slist *headers = curl_slist_append(nullptr, bearer_header.toStdString().data());


    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().data());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        qDebug() << "discordrpc::delete_asset:" << res;
        return false;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return true;
}

class discordrpc : public QObject {
    Q_OBJECT

public:
    discordrpc(mpd::player &t_player, discordapp t_dapp);

private slots:
    void mpd_connected();
    void player_update();
    void asset_ready(bool success, const QString &album, const asset &a);
    void asset_deleted(bool success, const asset &a);

private:
    mpd::player &m_player;
    discordapp m_dapp;
    QJsonObject m_assets;
    QMutex m_mutex;
    const QStringList RESERVED_ASSETS = {"unknown", "play", "pause", "stop"};

    [[nodiscard]] QVector<asset> get_assets() const;
    void delete_asset(const asset &a);
    void sync_assets();
    void save_asset_file();
};

class upload_worker : public QThread {
    Q_OBJECT

public:
    upload_worker(QString t_album, QString t_file, discordapp t_dapp, QObject *parent = nullptr);
    void run() override;

signals:
    void asset_uploaded(bool success, QString album, asset a);
private:
    QString m_album;
    QString m_file;
    discordapp m_dapp;
};

class delete_worker : public QThread {
    Q_OBJECT

public:
    delete_worker(asset t_asset, discordapp t_dapp, QObject *parent = nullptr);
    void run() override;

signals:
    void asset_deleted(bool success, asset a);

private:
    asset m_asset;
    discordapp m_dapp;
};
#else
#include "player.hpp"

struct discordapp {
    QString appid;
    QString token;
    bool coverart;
};

class discordrpc {
public:
    discordrpc(mpd::player &, discordapp) {}
};
#endif

#endif//PEMDAS_DISCORDRPC_HPP
