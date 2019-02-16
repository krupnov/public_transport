#ifndef PLANNER_STRUCTURES_H
#define PLANNER_STRUCTURES_H

#include <boost/cstdint.hpp>
#include <string>

namespace data_structures {
    struct agency_t {
        uint64_t id;
        std::string name;
        std::string url;
        std::string timezone;
    };
}

#endif //PLANNER_STRUCTURES_H
