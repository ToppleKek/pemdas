#include "musicpage.hpp"

#define URI_COLUMN 5

musicpage::musicpage(Ui::pemdas &t_ui, mpd::player &t_player) : m_ui(t_ui), m_player(t_player) {
    connect(&m_player, &mpd::player::database_change, this, &musicpage::update, Qt::QueuedConnection);
}

void musicpage::setup_ui() {
    m_ui.music_tree_widget->setColumnWidth(0, 25);
    m_ui.music_tree_widget->setColumnWidth(1, 275);
    m_ui.music_tree_widget->setColumnWidth(2, 225);
    m_ui.music_tree_widget->setColumnWidth(3, 225);
    m_ui.music_tree_widget->setColumnWidth(4, 50);
    m_ui.music_tree_widget->hideColumn(URI_COLUMN);
    m_ui.music_tree_widget->sortItems(1, Qt::AscendingOrder);

    connect(m_ui.music_tree_widget, &QTreeWidget::itemDoubleClicked, this, &musicpage::tree_item_double_clicked, Qt::QueuedConnection);
    connect(m_ui.music_tree_widget, &QTreeWidget::customContextMenuRequested, this, &musicpage::context_menu, Qt::QueuedConnection);
}

void musicpage::update() {
    m_ui.music_tree_widget->clear();

    std::vector<mpd::song> songs = m_player.all_songs();

    for (const auto &song : songs) {
        auto *item = new QTreeWidgetItem(QStringList() << "" <<
                                                                 QString::fromStdString(song.title()) <<
                                                                 QString::fromStdString(song.artist()) <<
                                                                 QString::fromStdString(song.album()) <<
                                                                 utils::s_to_mmss(song.duration()) <<
                                                                 QString::fromStdString(song.path()));


        if (!song.track().empty())
            item->setData(0, Qt::DisplayRole, std::stoi(song.track()));

        m_ui.music_tree_widget->addTopLevelItem(item);
    }
}

void musicpage::tree_item_double_clicked(QTreeWidgetItem *item, int column) {
    unsigned int pos = m_player.queue_next(item->text(URI_COLUMN).toStdString());
    m_player.play_pos(pos);
}

void musicpage::context_menu(const QPoint &pos) {
    QMenu menu;
//    QMenu playlist_menu;

    QAction play_next("Play Next", &menu);
    QAction append("Add to Queue", &menu);
//    QAction new_playlist("New Playlist...", &playlist_menu);

    connect(&play_next, &QAction::triggered, this, [&](bool checked) {
        QList<QTreeWidgetItem *> items = m_ui.music_tree_widget->selectedItems();

        for (const auto &item : items)
            m_player.queue_next(item->text(URI_COLUMN).toStdString());
    }, Qt::QueuedConnection);

    connect(&append, &QAction::triggered, this, [&](bool checked) {
        QList<QTreeWidgetItem *> items = m_ui.music_tree_widget->selectedItems();

        for (const auto &item : items)
            m_player.append_to_queue(item->text(URI_COLUMN).toStdString());
    }, Qt::QueuedConnection);

    menu.addAction(&play_next);
    menu.addAction(&append);
    menu.exec(m_ui.music_tree_widget->mapToGlobal(pos));
}
