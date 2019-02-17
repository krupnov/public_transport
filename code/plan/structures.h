#ifndef PLANNER_STRUCTURES_H
#define PLANNER_STRUCTURES_H

#include <boost/cstdint.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/greg_weekday.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace data_structures {
    namespace bg = boost::geometry;

    struct route_t;
    struct agency_t;
    struct service_t;
    struct service_exception_t;
    struct stop_t;
    struct transfer_t;
    struct trip_t;
    struct stop_time_t;

    template<typename T>
    using value_by_id = std::unordered_map<std::string, T>;

    using week_day = boost::gregorian::greg_weekday::weekday_enum;

    using route_ptr = std::shared_ptr<route_t>;
    using agency_ptr = std::shared_ptr<agency_t>;
    using service_ptr = std::shared_ptr<service_t>;
    using service_exception_ptr = std::shared_ptr<service_exception_t>;
    using stop_ptr = std::shared_ptr<stop_t>;
    using transfer_ptr = std::shared_ptr<transfer_t>;
    using trip_ptr = std::shared_ptr<trip_t>;
    using stop_time_ptr = std::shared_ptr<stop_time_t>;

    using date_t = boost::gregorian::date;
    using point_t = bg::model::point<double, 2, bg::cs::geographic<bg::degree> >;

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
        std::vector<trip_ptr> trips;
    };

    struct service_exception_t {
        date_t date;
        int type; // 1 - added as subs, 2 - interrupted
    };

    struct date_hasher_t {
        size_t operator()(date_t const& date) const {
            static boost::posix_time::ptime const epoch(boost::gregorian::date(1970, 1, 1));
            boost::posix_time::ptime time(date);
            return (time - epoch).total_milliseconds();
        }
    };

    struct service_t {
       std::string id;
       date_t start;
       date_t end;
       std::unordered_set<week_day> week_days;
       std::unordered_map<date_t, service_exception_ptr, date_hasher_t> exceptions;
       std::vector<trip_ptr> trips;
    };

    struct stop_t {
        std::string id;
        std::string name;
        point_t location;
        stop_ptr parent;
        std::vector<transfer_ptr> transfers;
        std::vector<stop_time_ptr> stop_times;
    };

    struct transfer_t {
        stop_ptr from;
        stop_ptr to;
        int type;
        boost::posix_time::time_duration duration;
    };

    struct trip_t {
        route_ptr route;
        service_ptr service;
        std::string id;
        std::string head_sign;
        std::string short_name;
        int direction;
        std::vector<stop_time_ptr> stop_times;
    };

    struct stop_time_t {
        stop_ptr stop;
        trip_ptr trip;
        int sequence;
        boost::posix_time::time_duration arrival;
        boost::posix_time::time_duration departure;
    };
}

#endif //PLANNER_STRUCTURES_H
