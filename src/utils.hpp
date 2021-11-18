#ifndef PEMDAS_UTILS_HPP
#define PEMDAS_UTILS_HPP

#include <QPixmap>
#include <QString>
#include <QDir>
#include <QBuffer>
#include <QStandardPaths>
#include <attachedpictureframe.h>
#include <regex>
#include <flacfile.h>
#include <id3v2tag.h>
#include <mpegfile.h>

#include <QDebug>

#include "mpd.hpp"

namespace utils {
    static QString s_to_mmss(unsigned int time) {
        unsigned int m = (int) (time / 60);

        QString minutes = QString::number(m);

        time %= 60;
        unsigned int s = time;

        QString seconds = QString::number(s);

        return QString(((m < 10) ? ("0" + minutes) : minutes) + ":" + ((s < 10) ? ("0" + seconds) : seconds));
    }

    static QString get_dir(const QString &path) {
        std::string s = path.toStdString();
        std::smatch m;
        std::regex e("^(.+)/");

        if (std::regex_search(s, m, e) && !m.empty())
            return QString::fromStdString(m.str(0));

        return QString();
    }

    static QString get_cover_file(const QString &dir_path) {
        QDir dir(dir_path);
        dir.setFilter(QDir::Files);

        std::smatch m;
        std::regex e("cover\\.(png|jpg|jpeg)", std::regex_constants::icase | std::regex_constants::nosubs);

        for (const auto &file : dir.entryList()) {
            std::string s = file.toStdString();
            if (std::regex_search(s, m, e) && !m.empty())
                return QString::fromStdString(m.str(0));
        }

        return QString();
    }

//    static void load_unknown() {
//        QFile unknown_cover(":/images/unknown.png");
//        unknown_cover.open(QIODevice::ReadOnly);
//        cover_map.insert("__pemdas__unknown__", unknown_cover.readAll().toBase64());
//    }

//    static bool cover_is_cached(const QString &album) {
//        QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
//
//        if (!dir.exists())
//            dir.mkpath(".");
//
//        return dir.exists(album + ".png");
//    }

//    static const QString &load_cached_cover(const QString &album) {
//        QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
//
//        if (!dir.exists())
//            dir.mkpath(".");
//
//        QPixmap image(dir.filePath(album + ".png"));
//        QBuffer imgbuf;
//
//        imgbuf.open(QIODevice::WriteOnly);
//        image.save(&imgbuf, "PNG", 100);
//
//        cover_map.insert(album, imgbuf.data().toBase64());
//
//        return cover_map[album];
//    }

    static QString get_cover(const QString &file, int w, int h, bool &has_cover) {
        QPixmap image;
        QString dir = get_dir(file);
        QString cover_file = get_cover_file(dir);

        if (!cover_file.isEmpty()) {
            has_cover = true;
            image.load(dir + cover_file);
            goto end;
        }

        if (file.endsWith(".mp3")) {
            TagLib::MPEG::File mpeg_file(file.toUtf8().data());
            TagLib::ID3v2::Tag *tag = mpeg_file.ID3v2Tag();

            if (!tag) {
                has_cover = false;
                image.load(":/images/unknown.png");
                goto end;
            }

            TagLib::ID3v2::FrameList list = tag->frameList("APIC");

            if (list.isEmpty()) {
                has_cover = false;
                image.load(":/images/unknown.png");
                goto end;
            }

            auto frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());

            image.loadFromData(reinterpret_cast<const uchar *>(frame->picture().data()), frame->picture().size());
            has_cover = true;
        } else if (file.endsWith(".flac")) {
            TagLib::FLAC::File flac_file(file.toUtf8().data());
            TagLib::List<TagLib::FLAC::Picture *> pictures = flac_file.pictureList();

            if (pictures.isEmpty()) {
                has_cover = false;
                image.load(":/images/unknown.png");
                goto end;
            }

            TagLib::ByteVector bv = pictures.front()->data();
            const char *d = bv.data();

            image.loadFromData(reinterpret_cast<const uchar *>(d), bv.size());
            has_cover = true;
        }

end:
        image = image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QBuffer imgbuf;

        imgbuf.open(QIODevice::WriteOnly);
        image.save(&imgbuf, "PNG");

        return imgbuf.data().toBase64();
    }

    static QString get_cover(const QString &file) {
        bool b;
        return get_cover(file, 256, 256, b);
    }
}

#endif //PEMDAS_UTILS_HPP
