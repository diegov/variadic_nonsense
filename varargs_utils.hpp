#ifndef H__TURNS__VARARGS_UTILS_HPP__
#define H__TURNS__VARARGS_UTILS_HPP__

#include <utility>
#include <tuple>
#include <type_traits>
#include <functional>

using namespace std;

namespace turns {
    // Skip tuple value
    
    template <size_t, typename>
    struct skip_type_holder;

    template <template <typename...> class Container>
    struct skip_type_holder<0, Container<> > {
        using type = Container<>;
    };

    template <template <typename...> class Container, typename Head, typename... Tail>
    struct skip_type_holder<0, Container<Head, Tail...> > {
        using type = Container<Head, Tail...>;
    };

    template <size_t k, template <typename...> class Container, typename Head, typename... Tail>
    struct skip_type_holder<k, Container<Head, Tail...> > {
        using type = typename skip_type_holder<k - 1, Container<Tail...> >::type; 
    };

    template <size_t, size_t, typename Source, typename Destination> struct set_tuple_value;

    template <size_t offset, typename Source, typename Destination>
    struct set_tuple_value<0, offset, Source, Destination> {
        static void set_value(Source &source, Destination &destination) {
        }
    };
        
    template <size_t k, size_t offset, typename Source, typename Destination>
    struct set_tuple_value {
        static void set_value(Source &source, Destination &destination) {
            get<k - 1>(destination) = get<k + offset - 1>(source);
            set_tuple_value<k - 1, offset, Source, Destination>::set_value(source, destination);
        }
    };

    template <size_t, typename Tuple> struct skip_tuple_value;

    template <size_t k, typename... Ts>
    struct skip_tuple_value<k, std::tuple<Ts...> > {
        using type = typename skip_type_holder<k, std::tuple<Ts...> >::type;
        
        static type get_value(std::tuple<Ts...> &source) {
            type retval;
            set_tuple_value<sizeof...(Ts) - k, k, std::tuple<Ts...>, type>::set_value(source, retval);
            return retval;
        }
    };

    // Revert variadic template arguments

    template <template <typename...> typename, typename...>
    struct revert_helper;

    template <template <typename> typename Template, typename Arg>
    struct revert_helper<Template, Arg> {
        using type = Template<Arg>;
    };

    template <template <typename...> typename Template, typename Head, typename... Tail>
    struct revert_helper<Template, Head, Tail...> {
    private:
        template <typename... XArgs>
        using tail_binding = Template<XArgs..., Head>;

    public:
        using type = typename revert_helper<tail_binding, Tail...>::type;
    };

    // Trim tuple value

    template <size_t, typename> struct trim_type_holder;

    template <size_t k, template <typename...> class Template, typename... Ts>
    struct trim_type_holder<k, Template<Ts...> > {
    private:
        typedef typename revert_helper<Template, Ts...>::type reversed;
    public:
        typedef typename skip_type_holder<k, reversed>::type type;
    };

    template <size_t, size_t, typename Source, typename Destination> struct trim_set_tuple_value;

    template <size_t source_index, typename Source, typename Destination>
    struct trim_set_tuple_value<source_index, 0, Source, Destination> {
        static void set_value(Source &source, Destination &destination) {
        }
    };
        
    template <size_t source_index, size_t destination_index, typename Source, typename Destination>
    struct trim_set_tuple_value {
        static void set_value(Source &source, Destination &destination) {
            get<destination_index - 1>(destination) = get<source_index>(source);
            trim_set_tuple_value<source_index + 1, destination_index - 1, Source, Destination>::set_value(source, destination);
        }
    };

    template <size_t, typename Tuple> struct trim_tuple_value;

    template <size_t k, typename... Ts>
    struct trim_tuple_value<k, std::tuple<Ts...> > {
        using type = typename trim_type_holder<k, std::tuple<Ts...> >::type;

        static type get_value(std::tuple<Ts...> &source) {
            type retval;
            trim_set_tuple_value<0, sizeof...(Ts) - k, std::tuple<Ts...>, type>::set_value(source, retval);
            return retval;
        }
    };

    // Add functions so that we can deduce the tuple argument when calling
    
    template <size_t k, typename Tuple>
    typename trim_type_holder<k, Tuple>::type trim_tuple(Tuple &source) {
        return trim_tuple_value<k, Tuple>::get_value(source);
    }

    template <size_t k, typename Tuple>
    typename skip_type_holder<k, Tuple>::type skip_tuple(Tuple &source) {
        return skip_tuple_value<k, Tuple>::get_value(source);
    }

    // CrossJoin
    
    template <template <typename> typename, typename...>
    struct CrossJoin;

    template <template <typename> typename Container,
              typename Current,
              typename... Previous>
    struct CrossJoin<Container, Current, Previous...> {
        using parent_type = CrossJoin<Container, Previous...>;
        
    public:
        CrossJoin(parent_type parent,
                  Container<Current> &values): parent(parent), values(values) { }
        
        CrossJoin<Container, Previous...> parent;

        template <typename... XArgs>
        using func_type = std::function<void(XArgs...)>;

        using receiver_type = typename revert_helper<func_type, Current, Previous...>::type;

        template <typename>
        struct func_unpacker;

        template <typename... Args>
        struct func_unpacker<std::function<void(Args...)> > {
            static std::function<void(Args...)> get_apply(receiver_type &receiver, Container<Current> &values) {
                return [&, receiver](Args... args) {
                    for (auto v : values) {
                        receiver(args..., v);
                    }                
                };                
            }
        };

        void apply(receiver_type receiver) {
            parent.apply(func_unpacker<typename parent_type::receiver_type>::get_apply(receiver, this->values));
        }

        template <typename Next>
        CrossJoin<Container, Next, Current, Previous...> join(Container<Next> &next) {
            CrossJoin<Container, Next, Current, Previous...> retval(*this, next);
            return retval;
        }
    private:
        Container<Current> &values;
    };

    template <template <typename> typename Container, typename Current>
    struct CrossJoin<Container, Current> {
    public:
        CrossJoin(Container<Current> &values): values(values) { }

        using receiver_type = std::function<void(Current)>;
        
        void apply(receiver_type receiver) {
            for (auto v : values) {
                receiver(v);
            }
        }

        template <typename Next>
        CrossJoin<Container, Next, Current> join(Container<Next> &next) {
            CrossJoin<Container, Next, Current> retval(*this, next);
            return retval;
        }
    private:
        Container<Current> &values;
    };

    template <template <typename> typename Container, typename T>
    CrossJoin<Container, T> cross_join(Container<T> &values) {
        CrossJoin<Container, T> retval(values);
        return retval;
    }
}

#endif
