#include "player.hpp"

#define PEMDAS_BEGIN_COMMANDS() begin_commands(__FUNCTION__)
#define PEMDAS_END_COMMANDS() end_commands(__FUNCTION__)

using namespace mpd;

player::player() :
    m_host{"localhost"},
    m_port{6600},
    m_timeout{5000} {}

player::player(std::string t_host, unsigned int t_port, unsigned int t_timeout) :
    m_host{std::move(t_host)},
    m_port{t_port},
    m_timeout{t_timeout} {}

void player::connect() {
    m_conn.reset(mpd_connection_new(m_host.c_str(), m_port, m_timeout), &mpd_connection_free);
    check_errors();

    m_fd = mpd_connection_get_fd(m_conn.get());
    check_errors();

    idle();

    emit connected();
}

void player::disconnect() {
    mpd_connection_free(m_conn.get());
    emit disconnected();
}

void player::set_host(std::string t_host) {
    m_host = std::move(t_host);
}

void player::set_port(unsigned int t_port) {
    m_port = t_port;
}

void player::set_timeout(unsigned int t_timeout) {
    m_timeout = t_timeout;
}

int player::fd() const {
    return m_fd;
}

void player::check_errors() const {
    mpd_error errcode = mpd_connection_get_error(m_conn.get());

    if (errcode == MPD_ERROR_SERVER) {
        throw server_error(mpd_connection_get_server_error(m_conn.get()),
                mpd_connection_get_error_message(m_conn.get()),
                false);
    } else if (errcode == MPD_ERROR_SYSTEM) {
        throw system_error(mpd_connection_get_system_error(m_conn.get()),
                mpd_connection_get_error_message(m_conn.get()),
                mpd_connection_clear_error(m_conn.get()));
    } else if (errcode != MPD_ERROR_SUCCESS) {
        throw client_error(mpd_connection_get_error(m_conn.get()),
                mpd_connection_get_error_message(m_conn.get()),
                true);
    }
}

void player::update() {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_update(m_conn.get(), nullptr))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::play_pos(unsigned int pos) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_play_pos(m_conn.get(), pos))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::play_next() {
    PEMDAS_BEGIN_COMMANDS();

    unsigned int pos;

    if (status().state() == MPD_STATE_STOP)
        pos = 0;
    else
        pos = mpd_song_get_pos(mpd_run_current_song(m_conn.get())) + 1;

    if (!mpd_run_play_pos(m_conn.get(), pos))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::insert_into_queue(unsigned int pos, const std::string &uri) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_add_id_to(m_conn.get(), uri.c_str(), pos))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::remove_from_queue(unsigned int pos) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_delete(m_conn.get(), pos))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::clear_queue() {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_clear(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();
}

unsigned int player::queue_next(const std::string &uri) {
    PEMDAS_BEGIN_COMMANDS();

    unsigned int pos;

    if (status().state() == MPD_STATE_STOP)
        pos = queue_songs().size();
    else
        pos = current_song().pos() + 1;

    if (!mpd_run_add_id_to(m_conn.get(), uri.c_str(), pos))
        check_errors();

    PEMDAS_END_COMMANDS();

    return pos;
}

void player::append_to_queue(const std::string &uri) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_add(m_conn.get(), uri.c_str()))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::play_pause() {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_toggle_pause(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::prev() {
    PEMDAS_BEGIN_COMMANDS();

    mpd::status s(m_conn.get());

    if (s.state() == MPD_STATE_STOP || s.state() == MPD_STATE_UNKNOWN)
        return;

    if (!mpd_run_previous(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::next() {
    PEMDAS_BEGIN_COMMANDS();

    mpd::status s(m_conn.get());

    if (s.state() == MPD_STATE_STOP || s.state() == MPD_STATE_UNKNOWN)
        return;

    if (!mpd_run_next(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::set_volume(unsigned int volume) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_set_volume(m_conn.get(), volume))
        check_errors();

    PEMDAS_END_COMMANDS();
}

void player::seek(float pos) {
    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_run_seek_current(m_conn.get(), pos, false))
        check_errors();

    PEMDAS_END_COMMANDS();
}

status player::status() {
    PEMDAS_BEGIN_COMMANDS();
    mpd::status s(m_conn.get());
    PEMDAS_END_COMMANDS();

    return s;
}

std::vector<song> player::all_songs() {
    std::vector<song> songs;
    mpd_entity *entity;

    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_send_list_all_meta(m_conn.get(), ""))
        check_errors();

    while ((entity = mpd_recv_entity(m_conn.get())) != nullptr) {
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
            songs.emplace_back(song(mpd_entity_get_song(entity)));

        mpd_entity_free(entity);
    }

    PEMDAS_END_COMMANDS();

    return songs;
}

std::vector<song> player::queue_songs() {
    std::vector<song> songs;
    mpd_entity *entity;

    PEMDAS_BEGIN_COMMANDS();
    //PEMDAS_BEGIN_COMMANDS();

    if (!mpd_send_list_queue_meta(m_conn.get()))
        check_errors();

    while ((entity = mpd_recv_entity(m_conn.get())) != nullptr) {
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
            songs.emplace_back(song{mpd_entity_get_song(entity)});

        mpd_entity_free(entity);
    }

    if (!mpd_response_finish(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();
    //PEMDAS_END_COMMANDS();

    return songs;
}

std::vector<std::string> player::playlists() {
    mpd_playlist *plist;
    std::vector<std::string> plists;

    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_send_list_playlists(m_conn.get()))
        check_errors();

    while ((plist = mpd_recv_playlist(m_conn.get())) != nullptr) {
        plists.emplace_back(mpd_playlist_get_path(plist));
        mpd_playlist_free(plist);
    }

    if (!mpd_response_finish(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();

    return plists;
}

std::vector<song> player::playlist_songs(const std::string &plist) {
    std::vector<song> songs;
    mpd_entity *entity;

    PEMDAS_BEGIN_COMMANDS();

    if (!mpd_send_list_playlist_meta(m_conn.get(), plist.c_str()))
        check_errors();

    while ((entity = mpd_recv_entity(m_conn.get())) != nullptr) {
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
            songs.emplace_back(song(mpd_entity_get_song(entity)));

        mpd_entity_free(entity);
    }

    if (!mpd_response_finish(m_conn.get()))
        check_errors();

    PEMDAS_END_COMMANDS();

    return songs;
}

song player::current_song() {
    PEMDAS_BEGIN_COMMANDS();
    mpd_song *sp = mpd_run_current_song(m_conn.get());

    if (!sp) {
        check_errors();
        PEMDAS_END_COMMANDS();

        return song{};
    }

    song s(sp);

    mpd_song_free(sp);
    PEMDAS_END_COMMANDS();

    return s;
}

std::string player::music_dir() {
    return "Z:/syncthing/Music Library";
//    PEMDAS_BEGIN_COMMANDS();
//
//    if (!mpd_send_command(m_conn.get(), "config", nullptr))
//        check_errors();
//
//    mpd_pair *pair = mpd_recv_pair_named(m_conn.get(), "music_directory");
//    std::string dir = pair ? pair->value : "";
//    mpd_return_pair(m_conn.get(), pair);
//    mpd_response_finish(m_conn.get());
//
//    return dir;
}

void player::begin_commands(const std::string &func) {
    if (!m_in_commands) {
        m_in_commands = true;
        m_issuer = func;
        handle_events(noidle());
    }
}

void player::end_commands(const std::string &func) {
    if (m_in_commands && m_issuer == func) {
        m_in_commands = false;
        m_issuer.clear();
        idle();
    }
}

int player::noidle() {
    int flags = 0;
    if (m_idle && mpd_send_noidle(m_conn.get())) {
        m_idle = false;
        flags = mpd_recv_idle(m_conn.get(), true);
        mpd_response_finish(m_conn.get());
        check_errors();
    }

    return flags;
}

void player::idle() {
    if (!m_idle) {
        mpd_send_idle(m_conn.get());
        check_errors();
    }

    m_idle = true;
}
void player::handle_events() {
    int flags = noidle();
    handle_events(flags);
    idle();
}

void player::handle_events(unsigned int flags) {
    //qDebug() << "EVENT!" << flags << mpd_idle_name(static_cast<mpd_idle>(flags));

    if (flags & MPD_IDLE_DATABASE) emit database_change();
    if (flags & MPD_IDLE_STORED_PLAYLIST) emit stored_playlist_update();
    if (flags & MPD_IDLE_QUEUE) emit queue_update();
    if (flags & MPD_IDLE_PLAYER) emit player_update();
    if (flags & MPD_IDLE_MIXER) emit mixer_update();
    if (flags & MPD_IDLE_OUTPUT) emit output_update();
    if (flags & MPD_IDLE_OPTIONS) emit options_update();
    if (flags & MPD_IDLE_UPDATE) emit database_update();
}
