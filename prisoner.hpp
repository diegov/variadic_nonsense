#ifndef H__TURNS_ROCK_PAPER_PRISONER_HPP
#define H__TURNS_ROCK_PAPER_PRISONER_HPP

#include <stddef.h>
#include <utility>
#include <vector>
#include <random>

using namespace std;

namespace turns::prisoner {
    using turn = pair<bool, bool>;
    using score = pair<int, int>;
    
    class Player {
    public:
        Player(default_random_engine &rnd, bool retaliate, bool forgive, double randomised);
        bool get_move(bool is_p1, vector<turn> &history);
    private:
        default_random_engine &rnd;
        bool retaliate;
        bool forgive;
        double randomised;

        bool has_been_wronged;
    };

    class PrisonersDilemma {
    public:
        void test(size_t runs);
    private:
        pair<double, double> play(size_t turn_count, Player &p1, Player &p2, bool print_debug);
        score get_score(bool p1_move, bool p2_move);
    };
}

#endif
