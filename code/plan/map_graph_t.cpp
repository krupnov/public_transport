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
        ds::stop_time_ptr stop_time;
        ds::transfer_ptr transfer;
        std::shared_ptr<list_t> parent;
    };

    std::vector<ds::path_leg_t> unwind(std::shared_ptr<list_t> const& list) {
        std::deque<ds::path_leg_t> deq;
        auto next = list;
        while (next != nullptr) {
            ds::path_leg_t leg;
            leg.arrival = next->date_time;
            leg.stop = next->stop;
            leg.transport = next->stop_time;
            leg.transfer = next->transfer;
            deq.push_front(std::move(leg));
            next = next->parent;
        }
        return std::vector<ds::path_leg_t>(deq.cbegin(), deq.cend());
    }

    ds::date_time_t date_with_other_time(ds::date_time_t date, ds::time_t time) {
        return ds::date_time_t(date.date(), time);
    }

    struct next_stop_t {
        ds::stop_ptr destination;
        ds::stop_ptr source;
        ds::transfer_ptr transfer;
        ds::stop_time_ptr transport;

        next_stop_t(
                ds::stop_ptr const& destination,
                ds::stop_ptr const& source,
                ds::transfer_ptr const& transfer,
                ds::stop_time_ptr const& transport) noexcept :
                destination(destination),
                source(source),
                transfer(transfer),
                transport(transport) {
        }

        bool operator<(next_stop_t const& that) const {
            return destination->id < that.destination->id;
        }
    };
}

namespace processing {

    using next_stop_with_time_t = std::pair<ds::date_time_t, next_stop_t>;

    map_graph_t::map_graph_t(
            ds::value_by_id<ds::trip_ptr> &&trips,
            ds::value_by_id<ds::stop_ptr> &&stops,
            std::vector<ds::stop_time_ptr> &&stop_times,
            ds::value_by_id<ds::service_ptr> &&services,
            ds::value_by_id<ds::route_ptr>&& routes) noexcept :
            trips(std::move(trips)), stops(std::move(stops)), stop_times(std::move(stop_times)),
            services(std::move(services)), routes(std::move(routes)) {
    }

     std::vector<ds::path_leg_t> map_graph_t::journey(
            std::string const& start, std::string const& finish, data_structures::date_time_t const& departure) const {
        if (stops.count(start) == 0 || stops.count(finish) == 0) {
            throw std::runtime_error("Unable to find start or finish stops by provided id");
        }
        auto const& source = stops.at(start);
        auto const& target = stops.at(finish);
        std::unordered_map<std::string, std::shared_ptr<list_t>> visited_stops;
        std::unordered_set<std::string> used_trips;
        std::priority_queue<
                next_stop_with_time_t, std::vector<next_stop_with_time_t>, std::greater<next_stop_with_time_t> > queue;
        queue.emplace(departure, next_stop_t(source, nullptr, nullptr, nullptr));
        while (!queue.empty()) {
            auto next = queue.top();
            queue.pop();
            if (visited_stops.count(next.second.destination->id) != 0) {
                continue;
            }
            auto step = std::make_shared<list_t>();
            step->stop = next.second.destination;
            step->transfer = next.second.transfer;
            step->stop_time = next.second.transport;
            step->date_time = next.first;
            if (next.second.source) {
                step->parent = visited_stops.at(next.second.source->id);
            }
            visited_stops.emplace(next.second.destination->id, step);
            if (next.second.destination->id == finish) {
                break;
            }
            auto const& s_t = next.second.destination->stop_times;

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
                            next_stop_t(
                                    (*next_stop_time_it)->stop,
                                    next.second.destination,
                                    nullptr,
                                    (*next_stop_time_it)));
                }
            }
            for (auto const& transfer : next.second.destination->transfers) {
                queue.emplace(next.first + transfer->duration,
                        next_stop_t(transfer->to, next.second.destination, transfer, nullptr));
            }
        }
        if (visited_stops.count(finish) == 0) {
            throw std::runtime_error("Unable to find connection");
        }
        return unwind(visited_stops.at(finish));
    }
}