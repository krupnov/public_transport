#include "parser.h"
#include "csv.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include <cassert>
#include <exception>
#include <unordered_map>
#include <iostream>
#include <utility>

namespace fs = boost::filesystem;
namespace ds = data_structures;

namespace {
    template<int column_count>
    using csv_reader = io::CSVReader<column_count, io::trim_chars<' '>, io::double_quote_escape<',','\"'>>;

    constexpr int AGENCIES_COLUMN_COUNT = 4;
    constexpr int ROUTES_COLUMN_COUNT = 6;
    constexpr int REGULAR_SERVICES_COLUMN_COUNT = 10;
    constexpr int EXCEPTIONAL_SERVICES_COLUMN_COUNT = 3;
    constexpr int STOPS_COLUMN_COUNT = 5;
    constexpr int TRANSFERS_COLUMN_COUNT = 4;
    constexpr int TRIPS_COLUMN_COUNT = 6;
    constexpr int STOP_TIMES_COLUMN_COUNT = 5;

    ds::value_by_id<ds::agency_ptr> parse_agencies(fs::path const& path) {
        ds::value_by_id<ds::agency_ptr> agencies;
        csv_reader<AGENCIES_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column, "agency_id", "agency_name", "agency_url", "agency_timezone");
        auto agency = std::make_shared<ds::agency_t>();
        while (reader.read_row(agency->id, agency->name, agency->url, agency->timezone)) {
            agencies.emplace(agency->id,  std::move(agency));
            agency = std::make_shared<ds::agency_t>();
        }
        return agencies;
    }

    bool try_get_table_path(fs::path const& directory, fs::path const& file, fs::path& result) {
        if (!fs::is_regular_file(directory / file)) {
            return false;
        }
        result = directory / file;
        return true;
    }

    fs::path get_table_path(fs::path const& directory, fs::path const& file) {
        fs::path result;
        if (!try_get_table_path(directory, file, result)) {
            throw std::runtime_error("No table found: " + file.string());
        }
        return result;
    }

    ds::value_by_id<ds::route_ptr> parse_routes(fs::path const& path, ds::value_by_id<ds::agency_ptr> const& agencies) {
        ds::value_by_id<ds::route_ptr> routes;
        csv_reader<ROUTES_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column,
                "route_id", "agency_id", "route_short_name", "route_long_name", "route_desc" ,"route_type");
        auto route = std::make_shared<ds::route_t>();
        std::string agency_id;
        while (reader.read_row(route->id, agency_id, route->short_name, route->long_name, route->desc, route->type)) {
            route->agency = agencies.at(agency_id);
            agencies.at(agency_id)->routes.push_back(route);
            routes.emplace(route->id, std::move(route));
            route = std::make_shared<ds::route_t>();
        }
        return routes;
    }

    ds::value_by_id<ds::service_ptr> parse_regular_services(fs::path const& path) {
        ds::value_by_id<ds::service_ptr> services;
        csv_reader<REGULAR_SERVICES_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column, "service_id", "monday", "tuesday", "wednesday", "thursday",
                "friday", "saturday", "sunday" , "start_date", "end_date");
        auto service = std::make_shared<ds::service_t>();
        int week_days[7];
        std::string start_date, end_date;
        while (reader.read_row(service->id, week_days[0], week_days[1], week_days[2], week_days[3], week_days[4],
                week_days[5], week_days[6], start_date, end_date)) {
            service->start = boost::gregorian::from_undelimited_string(start_date);
            service->end = boost::gregorian::from_undelimited_string(end_date);
            for (size_t i = 0 ; i < sizeof(week_days) / sizeof(week_days[0]) ; ++i) {
                if (week_days[i] == 1) {
                    service->week_days.insert(ds::week_day(i));
                }
            }
            services.emplace(service->id, std::move(service));
            service = std::make_shared<ds::service_t>();
        }
        return services;
    }

    void parse_exceptional_services(fs::path const& path, ds::value_by_id<ds::service_ptr> const& services) {
        csv_reader<EXCEPTIONAL_SERVICES_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column, "service_id", "date", "exception_type");
        auto service_exception = std::make_shared<ds::service_exception_t>();
        std::string date;
        std::string service_id;
        while (reader.read_row(service_id, date, service_exception->type)) {
            if (services.count(service_id) == 0) {
                assert(false);
                continue;
            }
            service_exception->date = boost::gregorian::from_undelimited_string(date);
            services.at(service_id)->exceptions.emplace(service_exception->date, std::move(service_exception));
            service_exception = std::make_shared<ds::service_exception_t>();
        }
    }

    ds::value_by_id<ds::stop_ptr> parse_stops(fs::path const& path) {
        csv_reader<STOPS_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column | io::ignore_missing_column, "stop_id", "stop_name", "stop_lat",
                "stop_lan", "parent_station");
        auto stop = std::make_shared<ds::stop_t>();
        ds::value_by_id<ds::stop_ptr> stops;
        double lat, lan;
        std::string parent_id;
        std::vector<std::pair<ds::stop_ptr, std::string>> with_parent;
        while (reader.read_row(stop->id, stop->name, lat, lan, parent_id)) {
            stop->location = ds::point_t(lat, lan);
            if (!parent_id.empty()) {
                with_parent.emplace_back(stop, parent_id);
            }
            stops.emplace(stop->id, std::move(stop));
            stop = std::make_shared<ds::stop_t>();
        }
        for (auto& update : with_parent) {
            update.first->parent = stops.at(update.second);
        }
        return stops;
    }

    void parse_transfers(fs::path const& path, ds::value_by_id<ds::stop_ptr> const& stops) {
       csv_reader<TRANSFERS_COLUMN_COUNT> reader(path.string());
       reader.read_header(io::ignore_extra_column, "from_stop_id", "to_stop_id", "transfer_type", "min_transfer_time");
       auto transfer = std::make_shared<ds::transfer_t>();
       std::string from, to;
       int time;
       while (reader.read_row(from, to, transfer->type, time)) {
           transfer->from = stops.at(from);
           transfer->to = stops.at(to);
           transfer->duration = boost::posix_time::seconds(time);
           stops.at(from)->transfers.emplace_back(std::move(transfer));
           transfer = std::make_shared<ds::transfer_t>();
       }
    }

    ds::value_by_id<ds::trip_ptr> parse_trips(fs::path const& path,
            ds::value_by_id<ds::route_ptr> const& routes, ds::value_by_id<ds::service_ptr> const& services) {
        csv_reader<TRIPS_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column, "route_id", "service_id", "trip_id", "trip_headsign",
                "trip_short_name", "direction_id");
        auto trip = std::make_shared<ds::trip_t>();
        ds::value_by_id<ds::trip_ptr> trips;
        std::string route_id, service_id;
        while (reader.read_row(route_id, service_id, trip->id, trip->head_sign, trip->short_name, trip->direction)) {
            trip->route = routes.at(route_id);
            trip->route->trips.push_back(trip);
            trip->service = services.at(service_id);
            trip->service->trips.push_back(trip);
            trips.emplace(trip->id, std::move(trip));
            trip = std::make_shared<ds::trip_t>();
        }
        return trips;
    }

    std::vector<ds::stop_time_ptr> parse_stop_times(fs::path const& path, ds::value_by_id<ds::trip_ptr> const& trips,
            ds::value_by_id<ds::stop_ptr> const& stops) {
        csv_reader<STOP_TIMES_COLUMN_COUNT> reader(path.string());
        reader.read_header(io::ignore_extra_column, "trip_id", "arrival_time", "departure_time", "stop_id",
                "stop_sequence");
        auto stop_time = std::make_shared<ds::stop_time_t>();
        std::vector<ds::stop_time_ptr> stop_times;
        std::string trip_id, stop_id, arrival, departure;
        while (reader.read_row(trip_id, arrival, departure, stop_id, stop_time->sequence)) {
            stop_time->arrival = boost::posix_time::duration_from_string(arrival);
            stop_time->departure = boost::posix_time::duration_from_string(departure);
            stop_time->trip = trips.at(trip_id);
            stop_time->trip->stop_times.push_back(stop_time);
            stop_time->stop = stops.at(stop_id);
            stop_time->stop->stop_times.push_back(stop_time);
            stop_times.emplace_back(std::move(stop_time));
            stop_time = std::make_shared<ds::stop_time_t>();
        }
        return stop_times;
    }
}

namespace util {
    void parse(std::string const& feed_directory) {
        if (!fs::is_directory(feed_directory)) {
            throw std::runtime_error("Feed directory is not directory: " + feed_directory);
        }
        fs::path feed(feed_directory);
        auto agencies = parse_agencies(get_table_path(feed, "agency.txt"));
        std::cout << "Agencies count: " << agencies.size() << std::endl;
        auto routes = parse_routes(get_table_path(feed, "routes.txt"), agencies);
        std::cout << "Routes count: " << routes.size() << std::endl;
        auto services = parse_regular_services(get_table_path(feed_directory, "calendar.txt"));
        std::cout << "Regular services count: " << services.size() << std::endl;
        fs::path exceptional_service_path;
        if (try_get_table_path(feed, "calendar_dates.txt", exceptional_service_path)) {
            parse_exceptional_services(exceptional_service_path, services);
            std::cout << "Service exceptions added" << std::endl;
        }
        auto stops = parse_stops(get_table_path(feed, "stops.txt"));
        std::cout << "Stops count: " << stops.size() << std::endl;
        fs::path transfers_path;
        if (try_get_table_path(feed, "transfers.txt", transfers_path)) {
            parse_transfers(transfers_path, stops);
            std::cout << "Transfers parsed" << std::endl;
        }
        auto trips = parse_trips(get_table_path(feed, "trips.txt"), routes, services);
        std::cout << "Trips count: " << trips.size() << std::endl;
        auto stop_times = parse_stop_times(get_table_path(feed, "stop_times.txt"), trips, stops);
        std::cout << "Stop times count: " << stop_times.size() << std::endl;
    }
}
