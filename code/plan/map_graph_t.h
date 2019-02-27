#ifndef PLANNER_MAP_GRAPH_T_H
#define PLANNER_MAP_GRAPH_T_H

#include "structures.h"

namespace processing {

    class map_graph_t {
        data_structures::value_by_id<data_structures::trip_ptr> trips;
        data_structures::value_by_id<data_structures::stop_ptr > stops;
        std::vector<data_structures::stop_time_ptr> stop_times;
        data_structures::value_by_id<data_structures::service_ptr> services;
        data_structures::value_by_id<data_structures::route_ptr> routes;
    public:
        map_graph_t(
                data_structures::value_by_id<data_structures::trip_ptr>&& trips,
                data_structures::value_by_id<data_structures::stop_ptr >&& stops,
                std::vector<data_structures::stop_time_ptr>&& stop_times,
                data_structures::value_by_id<data_structures::service_ptr>&& services,
                data_structures::value_by_id<data_structures::route_ptr>&& routes) noexcept;

        std::vector<data_structures::path_leg_t> journey(
                std::string const& start,
                std::string const& finish,
                data_structures::date_time_t const& departure) const;
    };

}


#endif //PLANNER_MAP_GRAPH_T_H
