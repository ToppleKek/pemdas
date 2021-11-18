#include "sidebar.hpp"

#define PLAYLIST_HEADER_ROW 6
#define OPTIONS_HEADER_ROW PLAYLIST_HEADER_ROW + m_num_playlists + 1
#define PLAYLIST_PAGE_INDEX 6
#define MPD_OPTIONS_ROW 7
#define PEMDAS_SETTINGS_ROW 8

sidebar::sidebar(Ui::pemdas &t_ui, mpd::player &t_player) : m_ui(t_ui), m_player(t_player) {
    connect(&m_player, &mpd::player::stored_playlist_update, this, &sidebar::playlist_update, Qt::QueuedConnection);
}

void sidebar::setup_ui() {
    connect(m_ui.tab_list_widget, &tablistwidget::currentItemChanged, this, &sidebar::current_item_changed, Qt::QueuedConnection);
    connect(m_ui.tab_list_widget, &tablistwidget::itemSelectionChanged, this, &sidebar::selection_changed, Qt::QueuedConnection);
}

void sidebar::update() {
    std::vector<std::string> playlists = m_player.playlists();
    m_num_playlists = playlists.size();

    int i = PLAYLIST_HEADER_ROW;
    for (const auto &playlist : playlists)
        m_ui.tab_list_widget->insertItem(++i, QString::fromStdString(playlist));
}

void sidebar::playlist_update() {
    update();
}

void sidebar::current_item_changed(QListWidgetItem *current, QListWidgetItem *previous) {
    if (previous)
        previous->setBackground(QColor::fromRgb(0, 0, 0, 0));

    if (current) {
        current->setBackground(QColor::fromRgb(0, 0, 0, 90));
        int row = m_ui.tab_list_widget->row(current);

        if (row > PLAYLIST_HEADER_ROW && row < OPTIONS_HEADER_ROW) {  // Playlist item
            m_ui.main_stacked_widget->setCurrentIndex(PLAYLIST_PAGE_INDEX);
            emit playlist_selected(current->text());
        } else if (row > OPTIONS_HEADER_ROW)
            m_ui.main_stacked_widget->setCurrentIndex(row == OPTIONS_HEADER_ROW + 1 ? MPD_OPTIONS_ROW : PEMDAS_SETTINGS_ROW);
        else
            m_ui.main_stacked_widget->setCurrentIndex(row);
    }
}

void sidebar::selection_changed() {
    if (!m_ui.tab_list_widget->selectedItems().isEmpty())
        m_ui.tab_list_widget->selectedItems().first()->setSelected(false);
}
