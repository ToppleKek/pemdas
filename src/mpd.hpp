#ifndef PEMDAS_MPD_HPP
#define PEMDAS_MPD_HPP

#include <exception>
#include <string>
#include <utility>
#include <memory>
#include <mpd/client.h>

namespace mpd {
    class client_error : public std::exception {
    public:
        client_error(mpd_error t_errcode, std::string t_errmsg, bool t_clearable) :
            m_errcode(t_errcode),
            m_errmsg(std::move(t_errmsg)),
            m_clearable(t_clearable) {}
        [[nodiscard]] const char *what() const noexcept override { return m_errmsg.c_str(); }
        [[nodiscard]] mpd_error errcode() const noexcept { return m_errcode; }
        [[nodiscard]] bool clearable() const noexcept { return m_clearable; }

    private:
        mpd_error m_errcode;
        std::string m_errmsg;
        bool m_clearable;
    };

    class server_error : public std::exception {
    public:
        server_error(mpd_server_error t_errcode, std::string t_errmsg, bool t_clearable) :
            m_errcode(t_errcode),
            m_errmsg(std::move(t_errmsg)),
            m_clearable(t_clearable) {}
        [[nodiscard]] const char *what() const noexcept override { return m_errmsg.c_str(); }
        [[nodiscard]] mpd_server_error errcode() const noexcept { return m_errcode; }
        [[nodiscard]] bool clearable() const noexcept { return m_clearable; }

    private:
        mpd_server_error m_errcode;
        std::string m_errmsg;
        bool m_clearable;
    };

    class system_error : public std::exception {
    public:
        system_error(int t_errcode, std::string t_errmsg, bool t_clearable) :
                m_errcode(t_errcode),
                m_errmsg(std::move(t_errmsg)),
                m_clearable(t_clearable) {}
        [[nodiscard]] const char *what() const noexcept override { return m_errmsg.c_str(); }
        [[nodiscard]] int errcode() const noexcept { return m_errcode; }
        [[nodiscard]] bool clearable() const noexcept { return m_clearable; }

    private:
        int m_errcode;
        std::string m_errmsg;
        bool m_clearable;
    };

    class status {
    public:
        explicit status(mpd_connection *t_conn) : m_conn(t_conn), m_status(mpd_run_status(m_conn), &mpd_status_free) {}

        [[nodiscard]] int volume() const { return mpd_status_get_volume(m_status.get()); }
        [[nodiscard]] bool repeat() const { return mpd_status_get_repeat(m_status.get()); }
        [[nodiscard]] bool random() const { return mpd_status_get_random(m_status.get()); }
        [[nodiscard]] bool single() const { return mpd_status_get_single(m_status.get()); }
        [[nodiscard]] bool consume() const { return mpd_status_get_consume(m_status.get()); }
        [[nodiscard]] unsigned int queue_length() const { return mpd_status_get_queue_length(m_status.get()); }
        [[nodiscard]] mpd_state state() const { return mpd_status_get_state(m_status.get()); }
        [[nodiscard]] unsigned int crossfade() const { return mpd_status_get_crossfade(m_status.get()); }
        [[nodiscard]] int current_song_pos() const { return mpd_status_get_song_pos(m_status.get()); }
        [[nodiscard]] int current_song_id() const { return mpd_status_get_song_id(m_status.get()); }
        [[nodiscard]] int next_song_pos() const { return mpd_status_get_next_song_pos(m_status.get()); }
        [[nodiscard]] int next_song_id() const { return mpd_status_get_next_song_id(m_status.get()); }
        [[nodiscard]] unsigned int elapsed_time() const { return mpd_status_get_elapsed_time(m_status.get()); }
        [[nodiscard]] unsigned int elapsed_time_ms() const { return mpd_status_get_elapsed_ms(m_status.get()); }
        [[nodiscard]] unsigned int total_time() const { return mpd_status_get_total_time(m_status.get()); }
        [[nodiscard]] unsigned int kbit_rate() const { return mpd_status_get_kbit_rate(m_status.get()); }

    private:
        mpd_connection *m_conn;
        std::shared_ptr<mpd_status> m_status;
    };

    class song {
    public:
        song() : m_title{}, m_artist{}, m_album{}, m_path{}, m_track{}, m_duration(0), m_pos(0), m_id(0) {}
        explicit song(const mpd_song *t_song) :
            m_title(mpd_song_get_tag(t_song, MPD_TAG_TITLE, 0) == nullptr ? mpd_song_get_uri(t_song) : mpd_song_get_tag(t_song, MPD_TAG_TITLE, 0)),
            m_artist(mpd_song_get_tag(t_song, MPD_TAG_ARTIST, 0) == nullptr ? "Unknown" : mpd_song_get_tag(t_song, MPD_TAG_ARTIST, 0)),
            m_album(mpd_song_get_tag(t_song, MPD_TAG_ALBUM, 0) == nullptr ? "Unknown" : mpd_song_get_tag(t_song, MPD_TAG_ALBUM, 0)),
            m_path(mpd_song_get_uri(t_song)),
            m_track(mpd_song_get_tag(t_song, MPD_TAG_TRACK, 0) == nullptr ? "" : mpd_song_get_tag(t_song, MPD_TAG_TRACK, 0)),
            m_duration(mpd_song_get_duration(t_song)),
            m_pos(mpd_song_get_pos(t_song)),
            m_id(mpd_song_get_id(t_song)) {}
        [[nodiscard]] std::string title() const { return m_title; }
        [[nodiscard]] std::string artist() const { return m_artist; }
        [[nodiscard]] std::string album() const { return m_album; }
        [[nodiscard]] std::string path() const { return m_path; }
        [[nodiscard]] std::string track() const { return m_track; }
        [[nodiscard]] unsigned int duration() const { return m_duration; }
        [[nodiscard]] unsigned int pos() const { return m_pos; }
        [[nodiscard]] unsigned int id() const { return m_id; }

        bool operator==(const song &other) const { return m_id == other.id() && m_pos == other.pos() && m_path == other.path(); }
        bool operator!=(const song &other) const { return !(*this == other); }

    private:
        std::string m_title;
        std::string m_artist;
        std::string m_album;
        std::string m_path;
        std::string m_track;
        unsigned int m_duration;
        unsigned int m_pos;
        unsigned int m_id;
    };
}
#endif //PEMDAS_MPD_HPP
