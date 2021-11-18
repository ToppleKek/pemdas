#ifdef PEMDAS_ENABLE_DISCORDRPC
#include "discordrpc.hpp"

discordrpc::discordrpc(mpd::player &t_player, discordapp t_dapp) : m_player(t_player), m_dapp(std::move(t_dapp)) {
    qRegisterMetaType<asset>();
    QDir app_data_dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    if (!app_data_dir.exists())
        app_data_dir.mkpath(".");

    QFile asset_file(app_data_dir.filePath("assets.json"));

    asset_file.open(QIODevice::ReadWrite);
    QByteArray arr{asset_file.readAll()};
    QJsonParseError err{};
    QJsonDocument asset_doc = QJsonDocument::fromJson(arr, &err);

    qDebug() << "discordrpc: asset_doc: err?:" << err.errorString() << asset_doc;

    if (asset_doc["assets"].isObject())
        m_assets = asset_doc["assets"].toObject();
    else
        m_assets = QJsonObject();

    //qDebug() << "discordrpc:" << m_assets.size() << m_assets.contains("なかよし!〇!なかよし!");
    sync_assets();

    Discord_Initialize(m_dapp.appid.toUtf8().data(), nullptr, 1, nullptr);
    connect(&m_player, &mpd::player::player_update, this, &discordrpc::player_update, Qt::QueuedConnection);
    connect(&m_player, &mpd::player::connected, this, &discordrpc::mpd_connected, Qt::QueuedConnection);
}

void discordrpc::mpd_connected() {
    player_update();
}

void discordrpc::player_update() {
    QMutexLocker lock(&m_mutex);

    mpd::status status = m_player.status();
    mpd_state state = status.state();
    DiscordRichPresence presence;
    memset(&presence, 0, sizeof(presence));

    if (state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN) {
        presence.details = "PEMDAS";
        presence.state = "Playback Stopped";
        presence.largeImageKey = "unknown";
        presence.smallImageKey = "stop";
        presence.smallImageText = "Stopped";
    } else {
        mpd::song s = m_player.current_song();
        QString album = QString::fromStdString(s.album());
        QString title = QString::fromStdString(s.title());
        QString artist = QString::fromStdString(s.artist());

        album.truncate(128);
        title.truncate(128);
        artist.truncate(128);

        // TODO: make my own discord rpc library
        QByteArray discord_is_stupid_title = title.toUtf8();
        QByteArray discord_is_stupid_album = album.toUtf8();
        QByteArray discord_is_stupid_artist = artist.toUtf8();

        const char *i_hate_discord_title = discord_is_stupid_title.data();
        const char *i_hate_discord_album = discord_is_stupid_album.data();
        const char *i_hate_discord_artist = discord_is_stupid_artist.data();

        presence.details = i_hate_discord_title;
        presence.state = i_hate_discord_artist;

        if (m_dapp.coverart && m_assets.contains(album)) {
            qDebug() << "album:" << album << "key:" << m_assets[album].toObject()["key"].toString().toUtf8().data();
            presence.largeImageKey = m_assets[album].toObject()["key"].toString().toUtf8().data();
        }
        else if (m_dapp.coverart) {
            qDebug() << "Getting upload_worker ready";
            // Set temp object so other player updates do not cause it to upload again while this worker is running
            QJsonObject obj;

            obj["key"] = "unknown";
            obj["id"] = "uploading";
            m_assets[album] = obj;

            presence.largeImageKey = "unknown";

            if (m_assets.size() >= 150) {
                long min = 0;
                asset min_asset;
                for (const auto &local_asset : m_assets.keys()) {
                    const asset a{m_assets[local_asset].toObject()["key"].toString(), m_assets[local_asset].toObject()["id"].toString()};
                    bool ok;
                    long n = a.key.toLong(&ok);

                    if (ok && min < n) {
                        min = n;
                        min_asset = a;
                    }
                }
                // TODO: might be a problem, consider another thread?
                delete_asset(min_asset);
            }

            auto *w = new upload_worker(album, QString::fromStdString(m_player.music_dir() + "/" + s.path()), m_dapp);

            connect(w, &upload_worker::asset_uploaded, this, &discordrpc::asset_ready, Qt::QueuedConnection);
            connect(w, &upload_worker::finished, w, &QObject::deleteLater, Qt::QueuedConnection);

            qDebug() << "Starting upload_worker";
            w->start();
        } else
            presence.largeImageKey = "unknown";

        presence.largeImageText = i_hate_discord_album;
        presence.smallImageText = (state == MPD_STATE_PLAY ? "Playing" : "Pause");
        presence.smallImageKey = (state == MPD_STATE_PLAY ? "play" : "pause");
        presence.startTimestamp = (state == MPD_STATE_PLAY ? std::time(nullptr) : 0);
        presence.endTimestamp = (state == MPD_STATE_PLAY ? std::time(nullptr) + s.duration() - status.elapsed_time() : 0);
    }

    Discord_UpdatePresence(&presence);
}

void discordrpc::asset_ready(bool success, const QString &album, const asset &a) {
    qDebug() << "asset is ready!" << success << album;
    if (success) {
        QJsonObject obj;

        obj["key"] = a.key;
        obj["id"] = a.id;
        m_assets[album] = obj;

        save_asset_file();
        player_update();
    } else {
        qDebug() << "failed to upload asset";
        m_assets.remove(album);
    }
}

void discordrpc::asset_deleted(bool success, const asset &a) {
    qDebug() << "asset has been deleted!" << success << a.key;

    if (!success)
        return;

    for (const auto &json_asset : m_assets.keys()) {
        const asset local_asset{m_assets[json_asset].toObject()["key"].toString(), m_assets[json_asset].toObject()["id"].toString()};

        if (local_asset == a) {
            m_assets.remove(json_asset);
            break;
        }
    }
}

QVector<asset> discordrpc::get_assets() const {
    QVector<asset> assets;
    CURL *curl = curl_easy_init();

    qDebug() << "discordrpc::get_assets: getting assets";

    if (!curl) {
        qDebug() << "discordrpc::get_assets: Failed to init curl";
        return assets;
    }

    QString url = "https://discordapp.com/api/oauth2/applications/" + m_dapp.appid + "/assets";
    QString response;

    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        qDebug() << "discordrpc::get_assets: " << res;
    else {
        QJsonParseError err{};
        qDebug() << "discordrpc::get_assets: CURLE_OK: " << response.toUtf8();
        QByteArray json = response.toUtf8().replace("\n0", "").replace('\n', "").replace('\r', "");
        QJsonArray json_response = QJsonDocument::fromJson(json, &err).array();
        qDebug() << "discordrpc::get_assets: size of json array:" << json_response.size() << "err:" << err.errorString();

        for (const auto &item : json_response) {
            qDebug() << "got asset:" << item;
            asset a;

            a.key = item.toObject()["name"].toString();
            a.id = item.toObject()["id"].toString();

            assets << a;
        }
    }

    curl_easy_cleanup(curl);

    return assets;
}

void discordrpc::delete_asset(const asset &a) {
    auto *w = new delete_worker(a, m_dapp);

    connect(w, &delete_worker::asset_deleted, this, &discordrpc::asset_deleted);
    connect(w, &delete_worker::finished, w, &QObject::deleteLater);

    w->start();
}

void discordrpc::sync_assets() {
    qDebug() << "discordrpc::sync_assets: syncing assets";
    QVector<asset> assets = get_assets();
    qDebug() << "discordrpc::sync_assets: got assets";

    for (const auto &local_asset : m_assets.keys()) {
        qDebug() << "checking asset:" << local_asset;

        if (m_assets[local_asset].toObject()["key"].toString().isEmpty()) {
            m_assets.remove(local_asset);
            continue;
        }

        const asset a{m_assets[local_asset].toObject()["key"].toString(), m_assets[local_asset].toObject()["id"].toString()};

        if (!assets.contains(a)) {
            qDebug() << "Removing asset:" << local_asset;
            m_assets.remove(local_asset);
        }
    }

    for (const auto &server_asset : assets) {
        qDebug() << "checking server asset:" << server_asset.key;

        for (const auto &json_asset : m_assets) {
            const asset a{json_asset.toObject()["key"].toString(), json_asset.toObject()["id"].toString()};

            if (a == server_asset)
                goto out;
        }

        if (RESERVED_ASSETS.contains(server_asset.key))
            continue;

        qDebug() << "missing server asset:" << server_asset.key << "deleting it now";
        delete_asset(server_asset);

out:
        continue;
    }

    save_asset_file();
}

void discordrpc::save_asset_file() {
    QJsonObject obj;
    obj["assets"] = m_assets;

    QJsonDocument doc(obj);
    QDir app_data_dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QFile asset_file(app_data_dir.filePath("assets.json"));

    asset_file.open(QIODevice::WriteOnly);

    asset_file.write(doc.toJson());
    asset_file.close();
}

upload_worker::upload_worker(QString t_album, QString t_file, discordapp t_dapp, QObject *parent)
    : QThread(parent),
      m_album(std::move(t_album)),
      m_file(std::move(t_file)),
      m_dapp(std::move(t_dapp)) {}

void upload_worker::run() {
    CURL *curl = curl_easy_init();
    QString url = "https://discordapp.com/api/oauth2/applications/" + m_dapp.appid + "/assets";
    QString bearer_header = "authorization: " + m_dapp.token;

    auto t = std::chrono::system_clock::now().time_since_epoch();
    QString key = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(t).count());

    if (!curl) {
        qDebug() << "upload_worker::run: Failed to init curl";
        return;
    }

    bool has_cover;
    QString cover = utils::get_cover(m_file, 512, 512, has_cover);

    if (!has_cover) {
        asset a{"unknown", "unknown"};
        emit asset_uploaded(true, m_album, a);
        return;
    }

    curl_slist *headers = nullptr;
    QString response;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, bearer_header.toStdString().c_str());

    qDebug() << "upload_worker: making request with auth header:" << bearer_header.toStdString().c_str() << '\n';

    auto pf = new char[key.size() + cover.size() + 53];
    std::sprintf(pf, R"({"name":"%s","type":1,"image":"data:image/png;base64,%s"})", key.toUtf8().data(), cover.toUtf8().data());

    //qDebug() << "upload_worker: making full request:" << pf << '\n';
    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    asset a{key, ""};

    if (res != CURLE_OK) {
        qDebug() << "upload_worker::run: " << res;
        emit asset_uploaded(false, m_album, a);
    } else {
        qDebug() << "upload_worker::run: uploaded asset!";
        qDebug() << "upload_worker::run: response:" << response;
        QJsonObject obj = QJsonDocument::fromJson(response.toUtf8()).object();
        a.id = obj["id"].toString();
        emit asset_uploaded(true, m_album, a);
    }

    delete[] pf;
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

delete_worker::delete_worker(asset t_asset, discordapp t_dapp, QObject *parent) : QThread(parent), m_asset(std::move(t_asset)), m_dapp(std::move(t_dapp)) {}

void delete_worker::run() {
    emit asset_deleted(send_delete_asset(m_asset, m_dapp.token, m_dapp.appid), m_asset);
}
#endif
