#ifndef H__TURNS__DEBUG_HPP__
#define H__TURNS__DEBUG_HPP__

#include <utility>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace turns {
    template <typename T>
    void print_results(map<T, vector<double> > &results, string parameter_name) {
        cout << "Results for " << parameter_name << endl;
        for (auto kv : results) {
            double total = 0;
            int count = 0;
            double min = 100000;
            double max = -100000;
            for (auto win_rate : kv.second) {
                if (win_rate < min) {
                    min = win_rate;
                }
                if (max < win_rate) {
                    max = win_rate;
                }
                total += win_rate;
                ++count;
            }

            cout << " value: " << kv.first << ", mean: " << (total / count) << ", min: " <<
                min << ", max: " << max << endl;
        }        
    }

}

#endif
