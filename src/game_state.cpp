#include "game_state.h"
#include <chrono>

GameState::GameState(const GameConfig& config) : config_(config) {
    players_.resize(config.num_players);
    for (int i = 0; i < config.num_players; i++) {
        players_[i].seat = i;
        players_[i].chips = config.starting_chips;
    }
    initial_player_count_ = config.num_players;
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned>(seed));
}

int GameState::swap_cost(Street street) const {
    return config_.swap_cost_multipliers[static_cast<int>(street)] * small_blind_;
}

int GameState::next_active_seat(int from) const {
    int n = config_.num_players;
    for (int i = 1; i <= n; i++) {
        int seat = (from + i) % n;
        if (!players_[seat].eliminated) {
            return seat;
        }
    }
    return from;
}

int GameState::small_blind_seat() const {
    if (players_remaining() == 2) {
        return dealer_seat_;
    }
    return next_active_seat(dealer_seat_);
}

int GameState::big_blind_seat() const {
    if (players_remaining() == 2) {
        return next_active_seat(dealer_seat_);
    }
    return next_active_seat(next_active_seat(dealer_seat_));
}

void GameState::advance_dealer() {
    dealer_seat_ = next_active_seat(dealer_seat_);
    hands_at_current_blind_++;

    // blinds double after each full revolution
    // a revolution = one hand per remaining player at the start of this blind level
    if (hands_at_current_blind_ >= players_remaining()) {
        small_blind_ *= 2;
        hands_at_current_blind_ = 0;
    }
}

void GameState::check_eliminations() {
    for (auto& p : players_) {
        if (!p.eliminated && p.chips <= 0) {
            p.eliminated = true;
            p.chips = 0;
        }
    }
}

bool GameState::is_game_over() const {
    return players_remaining() <= 1;
}

int GameState::winner_seat() const {
    for (const auto& p : players_) {
        if (!p.eliminated) return p.seat;
    }
    return -1;
}

int GameState::players_remaining() const {
    int count = 0;
    for (const auto& p : players_) {
        if (!p.eliminated) count++;
    }
    return count;
}
