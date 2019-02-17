#ifndef PLANNER_STRUCTURES_H
#define PLANNER_STRUCTURES_H

#include <boost/cstdint.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace data_structures {
    struct route_t;
    struct agency_t;


    template<typename T>
    using value_by_id = std::unordered_map<std::string, T>;

    using route_ptr = std::shared_ptr<route_t>;
    using agency_ptr = std::shared_ptr<agency_t>;

    struct agency_t {
        std::string id;
        std::string name;
        std::string url;
        std::string timezone;
        std::vector<route_ptr> routes;
    };

    struct route_t {
        std::string id;
        agency_ptr agency;
        std::string short_name;
        std::string long_name;
        std::string desc;
        int type;
    };
}

#endif //PLANNER_STRUCTURES_H
