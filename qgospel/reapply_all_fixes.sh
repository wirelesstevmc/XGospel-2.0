#!/bin/tcsh
# Re-apply all fixes from tonight's session

echo "Re-applying all player filtering fixes..."

# Already applied: BC/NR regex filter fix

# Fix 2: Add Hide Guests checkbox member variable
sed -i '705a\\tQCheckBox *hide_guests_checkbox;  // Filter to hide guest accounts (guestXXXX)' xgospel2_fixed.cpp

# Fix 3: Add Hide Guests checkbox initialization (after open_filter_checkbox)
sed -i '/connect(open_filter_checkbox.*applyOpenFilter);/a\\n\t\t// Hide Guests filter checkbox\n\t\thide_guests_checkbox = new QCheckBox("Hide Guests");\n\t\thide_guests_checkbox->setChecked(false);\n\t\tconnect(hide_guests_checkbox, \&QCheckBox::toggled, this, \&FixedPlayersWindow::applyHideGuestsFilter);' xgospel2_fixed.cpp

# Fix 4: Add Hide Guests checkbox to UI layout
sed -i '/filter_layout->addWidget(open_filter_checkbox);/a\\t\tfilter_layout->addWidget(hide_guests_checkbox);' xgospel2_fixed.cpp

# Fix 5: Add player status update function (after updatePlayerGameStatus location ~1157)
cat >> /tmp/status_funcs.txt << 'FUNCS'

	void updatePlayerGameStatus(const QString& player_name, const QString& game_nr, const QString& observing = QString()) {
		if (!players_model) return;
		qDebug() << "[PLAYER-STATUS] Updating" << player_name << "game:" << game_nr;
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (!name_item) continue;
			if (name_item->text() == player_name) {
				QStandardItem *pl_item = players_model->item(row, 3);
				if (pl_item) {
					pl_item->setText(game_nr.isEmpty() ? "--" : game_nr);
					qDebug() << "[PLAYER-STATUS] Updated" << player_name << "pl to:" << (game_nr.isEmpty() ? "--" : game_nr);
				}
				if (!observing.isEmpty()) {
					QStandardItem *ob_item = players_model->item(row, 4);
					if (ob_item) ob_item->setText(observing);
				}
				return;
			}
		}
		qDebug() << "[PLAYER-STATUS] Player" << player_name << "not found";
	}

	void resetAllPlayerGameStatuses() {
		if (!players_model) return;
		qDebug() << "[PLAYER-STATUS] Resetting all player game statuses to '--'";
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *pl_item = players_model->item(row, 3);
			if (pl_item) pl_item->setText("--");
			QStandardItem *ob_item = players_model->item(row, 4);
			if (ob_item) ob_item->setText("--");
		}
	}

	void applyHideGuestsFilter() {
		if (!players_model || !proxy_model) return;
		bool hide_guests = hide_guests_checkbox->isChecked();
		if (!players_table->isSortingEnabled()) {
			qDebug() << "[GUEST-FILTER] Skipping - sorting disabled";
			return;
		}
		qDebug() << "[GUEST-FILTER] Hide guests:" << hide_guests;
		int hidden_count = 0, visible_count = 0;
		for (int row = 0; row < players_model->rowCount(); ++row) {
			QStandardItem *name_item = players_model->item(row, 1);
			if (!name_item) continue;
			QString name = name_item->text();
			bool is_guest = name.startsWith("guest", Qt::CaseInsensitive);
			bool should_hide = hide_guests && is_guest;
			QModelIndex source_index = players_model->index(row, 0);
			QModelIndex proxy_index = proxy_model->mapFromSource(source_index);
			if (proxy_index.isValid()) {
				players_table->setRowHidden(proxy_index.row(), proxy_index.parent(), should_hide);
				if (should_hide) hidden_count++; else visible_count++;
			}
		}
		qDebug() << "[GUEST-FILTER] Visible:" << visible_count << "Hidden:" << hidden_count;
	}

	void reapplyHideGuestsFilter() {
		if (hide_guests_checkbox) applyHideGuestsFilter();
	}

FUNCS

sed -i '1157 r /tmp/status_funcs.txt' xgospel2_fixed.cpp

echo "Applied player status and guest filter functions"

# Fix 6: Add players_window_ref to FixedGamesWindow
sed -i '1946a\\tFixedPlayersWindow *players_window_ref;' xgospel2_fixed.cpp

# Fix 7: Initialize players_window_ref in FixedGamesWindow constructor
sed -i '1952s/newrating_enabled = false;/newrating_enabled = false;\n\t\tplayers_window_ref = nullptr;/' xgospel2_fixed.cpp

# Fix 8: Add setPlayersWindowRef and sync functions to FixedGamesWindow (after clearGames ~2175)
cat >> /tmp/games_funcs.txt << 'GFUNCS'

	void setPlayersWindowRef(FixedPlayersWindow *pw) {
		players_window_ref = pw;
		qDebug() << "[GAMES] Set players window reference";
	}

	void syncAllPlayerStatuses() {
		if (!players_window_ref || !games_model) return;
		qDebug() << "[GAMES] Syncing all player statuses...";
		players_window_ref->resetAllPlayerGameStatuses();
		for (int row = 0; row < games_model->rowCount(); ++row) {
			QStandardItem *game_nr_item = games_model->item(row, 0);
			QStandardItem *wname_item = games_model->item(row, 1);
			QStandardItem *bname_item = games_model->item(row, 3);
			if (!game_nr_item || !wname_item || !bname_item) continue;
			QString game_nr = game_nr_item->text();
			QString white_name = wname_item->text();
			QString black_name = bname_item->text();
			players_window_ref->updatePlayerGameStatus(white_name, game_nr);
			players_window_ref->updatePlayerGameStatus(black_name, game_nr);
		}
		qDebug() << "[GAMES] Player status sync complete for" << games_model->rowCount() << "games";
		players_window_ref->reapplyOpenFilter();
		players_window_ref->reapplyRankRangeFilter();
		players_window_ref->reapplyHideGuestsFilter();
	}

	void updateGamesCount() {
		int count = games_model->rowCount();
		setWindowTitle(QString("Games Online (%1)").arg(count));
	}

GFUNCS

sed -i '2175 r /tmp/games_funcs.txt' xgospel2_fixed.cpp

echo "Applied games window functions"

# Fix 9: Update player status when games are added
sed -i '/games_model->appendRow(row);/a\\t\tif (players_window_ref) {\n\t\t\tplayers_window_ref->updatePlayerGameStatus(game.wname, game.nr);\n\t\t\tplayers_window_ref->updatePlayerGameStatus(game.bname, game.nr);\n\t\t}\n\t\tupdateGamesCount();' xgospel2_fixed.cpp

# Fix 10: Update game count after clearGames
sed -i '/qDebug() << "\[GAMES\] Restored column widths"/a\\t\tupdateGamesCount();' xgospel2_fixed.cpp

# Fix 11: Link games_window to players_window (two locations ~5410, ~5659)
sed -i '5410a\\t\tgames_window->setPlayersWindowRef(players_window);' xgospel2_fixed.cpp
sed -i '5660a\\t\tgames_window->setPlayersWindowRef(players_window);' xgospel2_fixed.cpp

# Fix 12: Call syncAllPlayerStatuses after games list completes (~4100)
sed -i '/>>> COMPLETED: Parsed.*total games/i\\t\t\tgames_window->syncAllPlayerStatuses();\n\t\t\tgames_window->updateGamesCount();' xgospel2_fixed.cpp

# Fix 13: Add reapplyHideGuestsFilter calls (two locations)
sed -i '/this->reapplyRankRangeFilter();/a\\t\t\tthis->reapplyHideGuestsFilter();' xgospel2_fixed.cpp
sed -i '/players_window->reapplyRankRangeFilter();/a\\t\tplayers_window->reapplyHideGuestsFilter();' xgospel2_fixed.cpp

echo "All fixes re-applied successfully!"
echo "Now compiling..."
