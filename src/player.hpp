#ifndef PEMDAS_PLAYER_HPP
#define PEMDAS_PLAYER_HPP

#include <mpd/client.h>
#include <QObject>
#include <QDebug>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "mpd.hpp"

namespace mpd {
    class player : public QObject {
        Q_OBJECT

    public:
        player();
        player(std::string t_host, unsigned t_port, unsigned t_timeout);

        // Connection
        void connect();
        void disconnect();
        void set_host(std::string t_host);
        void set_port(unsigned int t_port);
        void set_timeout(unsigned int t_timeout);
        [[nodiscard]] int fd() const;
        void handle_events();
        void handle_events(unsigned int flags);

        // Commands
        void update();
        void play_pos(unsigned int pos);
        void play_next();
        void insert_into_queue(unsigned int pos, const std::string &uri);
        void remove_from_queue(unsigned int pos);
        void clear_queue();
        unsigned int queue_next(const std::string &uri);
        void append_to_queue(const std::string &uri);
        void play_pause();
        void prev();
        void next();
        void set_volume(unsigned int volume);
        void seek(float pos);
        mpd::status status();
        std::vector<mpd::song> all_songs();
        std::vector<mpd::song> queue_songs();
        std::vector<std::string> playlists();
        std::vector<mpd::song> playlist_songs(const std::string& plist);
        mpd::song current_song();
        std::string music_dir();

    signals:
        void connected();
        void disconnected();
        void database_change();
        void stored_playlist_update();
        void queue_update();
        void player_update();
        void mixer_update();
        void output_update();
        void options_update();
        void database_update();

    private:
        void begin_commands(const std::string &func);
        void end_commands(const std::string &func);
        int noidle();
        void idle();
        void check_errors() const;
        std::string m_host;
        unsigned int m_port = 0;
        unsigned int m_timeout = 0;
        std::shared_ptr<mpd_connection> m_conn;
        int m_fd = -1;
        bool m_idle = false;
        bool m_in_commands = false;
        std::string m_issuer{};
    };
}


#endif //PEMDAS_PLAYER_HPP
