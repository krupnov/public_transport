#include "parser.h"
#include "csv.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include <exception>
#include <unordered_map>
#include <iostream>

namespace fs = boost::filesystem;
namespace ds = data_structures;

namespace {
    template<int column_count>
    using csv_reader = io::CSVReader<column_count, io::trim_chars<' '>, io::double_quote_escape<',','\"'>>;

    constexpr int AGENCIES_COLUMN_COUNT = 4;
    constexpr int ROUTES_COLUMN_COUNT = 6;

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

    fs::path get_table_path(fs::path const& directory, fs::path const& file) {
        if (!fs::is_regular_file(directory / file)) {
            throw std::runtime_error("No table found: " + file.string());
        }
        return directory / file;
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
    }
}
