#include "map_graph_t.h"

#include <exception>
#include <queue>
#include <deque>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <utility>

namespace ds = data_structures;

namespace {
    struct list_t {
        ds::stop_ptr stop;
        ds::date_time_t date_time;
        std::shared_ptr<list_t> parent;
    };

    std::vector<std::pair<ds::stop_ptr, ds::date_time_t>> unwind(std::shared_ptr<list_t> const& list) {
        std::deque<std::pair<ds::stop_ptr, ds::date_time_t>> deq;
        auto next = list;
        while (next != nullptr) {
            deq.emplace_front(next->stop, next->date_time);
            next = next->parent;
        }
        return std::vector<std::pair<ds::stop_ptr, ds::date_time_t>>(deq.cbegin(), deq.cend());
    }

    ds::date_time_t date_with_other_time(ds::date_time_t const &date, ds::time_t const &time) {
        return ds::date_time_t(date.date()) + time;
    }
}

namespace processing {

    using next_stop_t = std::pair<ds::date_time_t, std::pair<ds::stop_ptr, ds::stop_ptr>>;

    map_graph_t::map_graph_t(
            ds::value_by_id<ds::trip_ptr> &&trips,
            ds::value_by_id<ds::stop_ptr> &&stops,
            std::vector<ds::stop_time_ptr> &&stop_times,
            ds::value_by_id<ds::service_ptr> &&services,
            ds::value_by_id<ds::route_ptr>&& routes) noexcept :
            trips(std::move(trips)), stops(std::move(stops)), stop_times(std::move(stop_times)),
            services(std::move(services)), routes(std::move(routes)) {
    }

     std::vector<std::pair<ds::stop_ptr, ds::date_time_t>> map_graph_t::journey(
            std::string const& start, std::string const& finish, data_structures::date_time_t const& departure) const {
        if (stops.count(start) == 0 || stops.count(finish) == 0) {
            throw std::runtime_error("Unable to find start or finish stops by provided id");
        }
        auto const& source = stops.at(start);
        auto const& target = stops.at(finish);
        std::unordered_map<std::string, std::shared_ptr<list_t>> visited_stops;
        std::unordered_set<std::string> used_trips;
        std::priority_queue<next_stop_t, std::vector<next_stop_t>, std::greater<next_stop_t> > queue;
        visited_stops.emplace(start, nullptr);
        queue.emplace(departure, std::make_pair(source, nullptr));
        while (!queue.empty()) {
            auto next = queue.top();
            queue.pop();
            if (visited_stops.count(next.second.first->id) != 0) {
                continue;
            }
            auto step = std::make_shared<list_t>();
            step->stop = next.second.first;
            step->date_time - next.first;
            if (next.second.second) {
                step->parent = visited_stops.at(next.second.second->id);
            }
            visited_stops.emplace(next.second.first->id, step);
            if (next.second.first->id == finish) {
                break;
            }
            auto const& s_t = next.second.first->stop_times;

            auto st_cmp = [&](ds::stop_time_ptr const& l, ds::date_time_t const& r) {
               return date_with_other_time(departure, l->departure) < r;
            };
            for (auto it =
                    std::lower_bound(s_t.cbegin(), s_t.cend(), next.first, st_cmp) ;
                    it != s_t.cend() ; ++it) {
                const auto& cur_trip = (*it)->trip;
                if (used_trips.count(cur_trip->id) != 0) {
                    continue;
                }
                used_trips.insert(cur_trip->id);
                auto next_stop_time_it = std::upper_bound(
                        cur_trip->stop_times.cbegin(), cur_trip->stop_times.cend(), (*it)->sequence,
                        [](int const& l, ds::stop_time_ptr const& r) {
                            return l < r->sequence;
                        });
                for ( ; next_stop_time_it != cur_trip->stop_times.cend() ; ++next_stop_time_it) {
                    queue.emplace(
                            date_with_other_time(departure, (*next_stop_time_it)->arrival),
                            std::make_pair((*next_stop_time_it)->stop, next.second.first));
                }
            }
        }
        if (visited_stops.count(finish) == 0) {
            throw std::runtime_error("Unable to find connection");
        }
        return unwind(visited_stops.at(finish));
    }
}