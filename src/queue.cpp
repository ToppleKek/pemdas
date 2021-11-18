#include "queue.hpp"

#define PLAY_ICON "\u25B6"

queue::queue(Ui::pemdas &t_ui, mpd::player &t_player) : m_ui(t_ui), m_player(t_player) {
    connect(&m_player, &mpd::player::queue_update, this, &queue::queue_update, Qt::QueuedConnection);
    connect(&m_player, &mpd::player::player_update, this, &queue::player_update, Qt::QueuedConnection);
}

void queue::setup_ui() {
    connect(m_ui.queue_tree_widget, &QTreeWidget::itemDoubleClicked, this, &queue::item_double_clicked, Qt::QueuedConnection);
    connect(m_ui.queue_tree_widget, &QTreeWidget::customContextMenuRequested, this, &queue::context_menu, Qt::QueuedConnection);

    m_ui.queue_tree_widget->header()->setSectionResizeMode(QHeaderView::Fixed);
    m_ui.queue_tree_widget->setColumnWidth(0, 25);
}

void queue::update() {
    const std::vector<mpd::song> songs = m_player.queue_songs();
    const mpd::song current_song = m_player.current_song();

    m_ui.queue_tree_widget->clear();

    int i = 0;
    for (const auto &song : songs) {
        auto *text_label = new QLabel(QString::fromStdString(song.title()) + "<br><small><i>By</i> " +
                             QString::fromStdString(song.artist()) + "<br><i>On</i> " +
                             QString::fromStdString(song.album()) + "<br>" +
                             utils::s_to_mmss(song.duration()) + "</small>");

        auto *ti = new QTreeWidgetItem;
        ti->setSizeHint(1, text_label->sizeHint());

        m_ui.queue_tree_widget->addTopLevelItem(ti);
        m_ui.queue_tree_widget->setItemWidget(ti, 1, text_label);

        if (song == current_song) {
            m_ui.queue_tree_widget->setItemWidget(ti, 0, new QLabel(QString("<center>") + PLAY_ICON + "</center>"));
            m_ui.queue_tree_widget->scrollToItem(ti);
            m_current_index = i;
        }

        ++i;
    }
}

void queue::player_update() {
    const mpd_state state = m_player.status().state();
    const mpd::song song = m_player.current_song();

    switch (state) {
        case MPD_STATE_PLAY:
        case MPD_STATE_PAUSE:
            m_ui.queue_tree_widget->setItemWidget(m_ui.queue_tree_widget->topLevelItem(m_current_index), 0, nullptr);
            m_current_index = song.pos();
            m_ui.queue_tree_widget->setItemWidget(m_ui.queue_tree_widget->topLevelItem(m_current_index), 0, new QLabel(QString("<center>") + PLAY_ICON + "</center>"));
            break;
        case MPD_STATE_STOP:
        case MPD_STATE_UNKNOWN:
            m_ui.queue_tree_widget->setItemWidget(m_ui.queue_tree_widget->topLevelItem(m_current_index), 0, nullptr);
    }
}

void queue::queue_update() {
    update();
}

void queue::item_double_clicked() {
    m_player.play_pos(m_ui.queue_tree_widget->currentIndex().row());
}

void queue::context_menu(const QPoint &pos) {
    QMenu menu;

    QAction remove("Remove", &menu);
    QAction clear("Clear Queue", &menu);

    connect(&remove, &QAction::triggered, this, [&](bool checked) {
        QList<QTreeWidgetItem *> items = m_ui.queue_tree_widget->selectedItems();

        for (const auto &item : items) {
            m_player.remove_from_queue(m_ui.queue_tree_widget->indexOfTopLevelItem(item));
            update();
        }
    }, Qt::QueuedConnection);

    connect(&clear, &QAction::triggered, this, [&](bool checked) {
        m_player.clear_queue();
    });

    menu.addAction(&remove);
    menu.addSeparator();
    menu.addAction(&clear);

    menu.exec(m_ui.queue_tree_widget->mapToGlobal(pos));
}
