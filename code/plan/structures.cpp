#include "structures.h"

namespace data_structures {
    bool stop_time_cmp(stop_time_ptr const& l, stop_time_ptr const& r) {
        return l->departure < r->departure;
    }
}
