#include "parser.h"
#include "csv.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include <exception>
#include <unordered_map>

namespace fs = boost::filesystem;

namespace {
    constexpr int AGENCIES_COLUMN_COUNT = 4;

    std::unordered_map<std::string, std::shared_ptr<data_structures::agency_t>> parse_agencies(fs::path const& path) {
        std::unordered_map<std::string, std::shared_ptr<data_structures::agency_t>> agencies;
        io::CSVReader<AGENCIES_COLUMN_COUNT, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> reader(
                path.string());
        reader.read_header(io::ignore_extra_column, "agency_id", "agency_name", "agency_url", "agency_timezone");
        auto agency = std::make_shared<data_structures::agency_t>();
        while (reader.read_row(agency->id, agency->name, agency->url, agency->timezone)) {
            agencies.emplace(agency->id,  std::move(agency));
            agency = std::make_shared<data_structures::agency_t>();
        }
        return agencies;
    }
}

namespace util {
    void parse(std::string const& feed_directory) {
        if (!fs::is_directory(feed_directory)) {
            throw std::runtime_error("Feed directory is not directory: " + feed_directory);
        }
        fs::path feed(feed_directory);
        fs::path agency_file("agency.txt");
        if (!fs::is_regular_file(feed_directory / agency_file)) {
            throw std::runtime_error("No agency.txt file found");
        }
        auto agencies = parse_agencies(feed_directory / agency_file);
    }
}
