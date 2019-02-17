#ifndef PLANNER_STRUCTURES_H
#define PLANNER_STRUCTURES_H

#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/greg_weekday.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace data_structures {
    struct route_t;
    struct agency_t;
    struct service_t;

    template<typename T>
    using value_by_id = std::unordered_map<std::string, T>;

    using week_day = boost::gregorian::greg_weekday::weekday_enum;

    using route_ptr = std::shared_ptr<route_t>;
    using agency_ptr = std::shared_ptr<agency_t>;
    using service_ptr = std::shared_ptr<service_t>;

    using date_t = boost::gregorian::date;

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

    struct service_t {
       std::string id;
       date_t start; // the only date for exceptions
       date_t end;
       int exception_type; //-1 for normal weekly service
       std::unordered_set<week_day> week_days;
    };
}

#endif //PLANNER_STRUCTURES_H
