#ifndef PLANNER_MAP_GRAPH_T_H
#define PLANNER_MAP_GRAPH_T_H

#include "structures.h"

#include <vector>

namespace processing {

    class map_graph_t {
    public:
        map_graph_t(
                data_structures::value_by_id<data_structures::trip_ptr>&& trips,
                data_structures::value_by_id<data_structures::stop_ptr >&& stops,
                std::vector<data_structures::stop_time_ptr>&& stop_times,
                data_structures::value_by_id<data_structures::transfer_ptr>&& transfers,
                data_structures::value_by_id<data_structures::service_ptr>&& services) noexcept;
    };

}


#endif //PLANNER_MAP_GRAPH_T_H
