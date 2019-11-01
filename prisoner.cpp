#include "prisoner.hpp"
#include <iostream>
#include <functional>
#include <vector>
#include "varargs_utils.hpp"
#include "stats.hpp"
#include "debug.hpp"

using namespace std;
using namespace turns;

namespace turns::prisoner {
    Player::Player(default_random_engine &rnd, bool retaliate, bool forgive, double randomised):
        rnd(rnd), retaliate(retaliate), forgive(forgive), randomised(randomised), has_been_wronged(false) { }

    bool Player::get_move(bool is_p1, vector<turn> &history) {
        if (history.empty()) {
            if (this->randomised > 0.0) {
                uniform_int_distribution<size_t> choice(0, 1);
                return choice(rnd) != 0;
            } else {
                return false;
            }
        }

        auto &previous = history[history.size() - 1];

        bool previous_self;
        bool previous_opponent;
        if (is_p1) {
            previous_self = previous.first;
            previous_opponent = previous.second;
        } else {
            previous_self = previous.second;
            previous_opponent = previous.first;
        }

        this->has_been_wronged = this->has_been_wronged | previous_opponent;

        bool must_retaliate;
        if (this->forgive) {
            must_retaliate = previous_opponent;
        } else {
            must_retaliate = this->has_been_wronged;
        }
        
        if (this->randomised > 0.0) {
            size_t limit = static_cast<size_t>(100 * this->randomised);
            if (must_retaliate) {
                limit += 100;
            }
            
            uniform_int_distribution<size_t> choice(0, limit);
            return choice(rnd) > limit / 2;
        } else {
            return must_retaliate;
        }
    }

    using datarow = tuple<bool, bool, bool, bool, double, double>;

    void PrisonersDilemma::test(size_t turn_count) {
        default_random_engine rnd;

        vector<pair<datarow, pair<double, double>>> results;

        vector<bool> true_false = { true, false };
        vector<double> random_levels = { 0.0, 0.25, 0.5, 0.75, 1.0 };
        cross_join(true_false)
            .join(true_false)
            .join(true_false)
            .join(true_false)
            .join(random_levels)
            .join(random_levels).apply([&](bool p1_retaliate, bool p2_retaliate,
                                        bool p1_forgive, bool p2_forgive,
                                        double p1_randomised, double p2_randomised) {

                Player p1(rnd, p1_retaliate, p1_forgive, p1_randomised);
                Player p2(rnd, p2_retaliate, p2_forgive, p2_randomised);

                pair<double, double> result = play(turn_count, p1, p2, false);

                auto key = make_tuple(p1_retaliate, p2_retaliate, p1_forgive, p2_forgive,
                                      p1_randomised, p2_randomised);

                results.push_back(make_pair(key, result));
            });

        function<double(pair<double, double>&)> p1_score = [](pair<double, double> &v) { return v.first; };
        function<double(pair<double, double>&)> p2_score = [](pair<double, double> &v) { return v.second; };
        
        auto p1_stats_retaliate = turns::get_stats<0>(results, p1_score);
        print_results(p1_stats_retaliate, "P1 retaliate");

        auto p2_stats_retaliate = turns::get_stats<1>(results, p2_score);
        print_results(p2_stats_retaliate, "P2 retaliate");

        auto p1_stats_forgive = turns::get_stats<2>(results, p1_score);
        print_results(p1_stats_forgive, "P1 forgive");

        auto p2_stats_forgive = turns::get_stats<3>(results, p2_score);
        print_results(p2_stats_forgive, "P2 forgive");

        auto p1_stats_randomise = turns::get_stats<4>(results, p1_score);
        print_results(p1_stats_randomise, "P1 randomise");

        auto p2_stats_randomise = turns::get_stats<5>(results, p2_score);
        print_results(p2_stats_randomise, "P2 randomise");
    }

    score PrisonersDilemma::get_score(bool p1_move, bool p2_move) {
        if (p1_move) {
            if (p2_move) {
                return make_pair(-1, -1);
            } else {
                return make_pair(2, -2);
            }
        } else {
            if (p2_move) {
                return make_pair(-2, 2);
            } else {
                return make_pair(0, 0);
            }
        }
    }

    pair<double, double> PrisonersDilemma::play(size_t turn_count, Player &p1, Player &p2, bool print_debug) {
        vector<score> results;
        vector<turn> history;
        
        for (size_t j = 0; j < turn_count; ++j) {
            bool p1_move = p1.get_move(true, history);
            bool p2_move = p2.get_move(false, history);

            score result = get_score(p1_move, p2_move);
            if (print_debug) {
                cout << "P1 " << p1_move << " P2 " << p2_move <<
                    " Score (" << result.first << ", " << result.second << ")" << endl;
            }

            results.push_back(result);
            history.emplace_back(p1_move, p2_move);
        }

        double result_1 = 0.0;
        double result_2 = 0.0;
        size_t count = results.size();
        for (auto r : results) {
            result_1 += r.first;
            result_2 += r.second;
        }

        return make_pair(result_1 / count, result_2 / count);
    }
}
