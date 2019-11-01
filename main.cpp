#include <iostream>
#include <sstream>
#include "prisoner.hpp"

using namespace std;

using namespace turns;

int main(int argc, char **argv) {
    if (argc >= 2) {
        std::istringstream iss(argv[1]);
        int total_turns;

        if (iss >> total_turns) {
            prisoner::PrisonersDilemma dilemma;
            dilemma.test(total_turns);
            return 0;
        }
    }

    cerr << "Usage: " << argv[0] << " number_of_runs:int" << endl;
    return 1;
}
