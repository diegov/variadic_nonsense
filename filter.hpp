#ifndef H__TURNS__FILTER_HPP__
#define H__TURNS__FILTER_HPP__

#include <functional>
#include <utility>

using namespace std;

namespace turns {
    template <template <typename> typename Container, typename T>
    pair<typename Container<T>::iterator, typename Container<T>::iterator> filter(Container<T> &source, function<bool(const T&)> predicate) {
        return make_pair(source.begin(), source.end());
    }
}

#endif
