#include "playlistpage.hpp"
#include <QMenu>

#define URI_COLUMN 6

playlistpage::playlistpage(Ui::pemdas &t_ui, mpd::player &t_player, sidebar &t_sidebar) : m_ui(t_ui), m_player(t_player), m_sidebar(t_sidebar) {
    connect(&m_sidebar, &sidebar::playlist_selected, this, &playlistpage::load_playlist, Qt::QueuedConnection);
}

void playlistpage::setup_ui() {
    m_ui.playlist_tree_widget->setColumnWidth(0, 50);
    m_ui.playlist_tree_widget->setColumnWidth(1, 50);
    m_ui.playlist_tree_widget->setColumnWidth(2, 275);
    m_ui.playlist_tree_widget->setColumnWidth(3, 225);
    m_ui.playlist_tree_widget->setColumnWidth(4, 225);
    m_ui.playlist_tree_widget->setColumnWidth(5, 50);
    m_ui.playlist_tree_widget->hideColumn(URI_COLUMN);
    m_ui.playlist_tree_widget->sortItems(0, Qt::AscendingOrder);

    connect(m_ui.playlist_tree_widget, &QTreeWidget::itemDoubleClicked, this, &playlistpage::tree_item_double_clicked, Qt::QueuedConnection);
    connect(m_ui.playlist_tree_widget, &QTreeWidget::customContextMenuRequested, this, &playlistpage::context_menu, Qt::QueuedConnection);
}

void playlistpage::tree_item_double_clicked(QTreeWidgetItem *item, int) {
    unsigned int pos = m_player.queue_next(item->text(URI_COLUMN).toStdString());
    m_player.play_pos(pos);
}

void playlistpage::context_menu(const QPoint &pos) {
    qDebug() << "playlistpage: context menu requested";
    QMenu menu;
//    QMenu playlist_menu;

    QAction play_next("Play Next", &menu);
    QAction append("Add to Queue", &menu);
//    QAction new_playlist("New Playlist...", &playlist_menu);

    connect(&play_next, &QAction::triggered, this, [&](bool checked) {
        QList<QTreeWidgetItem *> items = m_ui.playlist_tree_widget->selectedItems();

        for (const auto &item : items)
            m_player.queue_next(item->text(URI_COLUMN).toStdString());
    }, Qt::QueuedConnection);

    connect(&append, &QAction::triggered, this, [&](bool checked) {
        QList<QTreeWidgetItem *> items = m_ui.playlist_tree_widget->selectedItems();

        for (const auto &item : items)
            m_player.append_to_queue(item->text(URI_COLUMN).toStdString());
    }, Qt::QueuedConnection);

    menu.addAction(&play_next);
    menu.addAction(&append);
    menu.exec(m_ui.playlist_tree_widget->mapToGlobal(pos));
}

void playlistpage::load_playlist(const QString &playlist) {
    if (playlist == m_current_playlist)
        return;

    m_current_playlist = playlist;
    std::vector<mpd::song> songs = m_player.playlist_songs(playlist.toStdString());
    m_ui.playlist_tree_widget->clear();

    int i = 1;
    for (const auto &song : songs) {
        auto *item = new QTreeWidgetItem(QStringList() << "" << "" <<
                                                       QString::fromStdString(song.title()) <<
                                                       QString::fromStdString(song.artist()) <<
                                                       QString::fromStdString(song.album()) <<
                                                       utils::s_to_mmss(song.duration()) <<
                                                       QString::fromStdString(song.path()));

        item->setData(0, Qt::DisplayRole, i++);

        if (!song.track().empty())
            item->setData(1, Qt::DisplayRole, std::stoi(song.track()));

        m_ui.playlist_tree_widget->addTopLevelItem(item);
    }

}
