#ifndef H__TURNS__STATS_HPP__
#define H__TURNS__STATS_HPP__

#include <map>
#include <vector>
#include <tuple>
#include <utility>
#include <functional>
#include "varargs_utils.hpp"

using namespace std;

namespace turns {
    template <size_t index, typename... Args>
    struct result_types {
        using Left = typename trim_type_holder<sizeof...(Args) - index, std::tuple<Args...> >::type;
        using Right = typename skip_type_holder<index + 1, std::tuple<Args...> >::type;
        using Key = pair<Left, Right>;
        using Element = typename tuple_element<index, std::tuple<Args...> >::type;
    };

    template <size_t index, typename... Args>
    typename result_types<index, Args...>::Key
    make_key(std::tuple<Args...> source) {
        return make_pair(turns::trim_tuple<sizeof...(Args) - index>(source),
                         turns::skip_tuple<index + 1>(source));
    }

    template <size_t index, typename... Args>
    map<typename result_types<index, Args...>::Element, vector<double> >
    get_stats(vector<pair<tuple<Args...>, double> > &data) {
        return get_stats<index, double, Args...>(data, [](double &v) { return v; });
    }

    template <size_t index, typename Result, typename... Args>
    map<typename result_types<index, Args...>::Element, vector<double> >
    get_stats(vector<pair<tuple<Args...>, Result> > &data, function<double(Result&)> fetcher) {
        typedef typename result_types<index, Args...>::Element param_type;
        typedef typename result_types<index, Args...>::Key key_type;

        map<key_type, map<param_type, double> > results;
        
        for (pair<tuple<Args...>, Result> &value : data) {
            key_type key = make_key<index>(value.first);
                                 
            if (results.find(key) == results.end()) {
                results.emplace(key, map<param_type, double>());
            }

            map<param_type, double> &result = results[key];
            
            auto score = fetcher(value.second);
            param_type param_value = get<index>(value.first);

            result[param_value] = score;
        }

        map<param_type, vector<double> > retval;
        for (auto &isolated : results) {
            double total = 0.0;
            for (auto &kv : isolated.second) {
                total += kv.second;
            }

            double mean = total / isolated.second.size();

            for (auto &kv : isolated.second) {
                param_type key = kv.first;
                double advantage = kv.second - mean;
                if (retval.find(key) == retval.end()) {
                    retval[key] = vector<double>();
                }
                retval[key].push_back(advantage);
            }
        }

        return retval;
    }
}

#endif
