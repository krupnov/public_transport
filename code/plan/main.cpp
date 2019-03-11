#include "parser.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <exception>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    po::options_description desc("Options");
    desc.add_options()
            ("feed_directory", po::value<std::string>()->required(), "Enter feed directory")
            ("help", "Print help messages");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    try {
        po::notify(vm);
        auto feed_directory = vm["feed_directory"].as<std::string>();
        std::cout << "Parsing feed" << std::endl;
        auto map = util::parse(feed_directory);
        std::cout << "Enter start id than stop id and than departure date time each in separate line" << std::endl;
        std::cout << "For exit enter 'q'" << std::endl;
        while(true) {
            std::string start, finish, departure;
            std::getline(std::cin , start);
            if (start == "q") {
                break;
            }
            std::getline(std::cin, finish);
            std::getline(std::cin, departure);
            try {
                for (auto const& leg : map.journey(start, finish, boost::posix_time::time_from_string(departure))) {
                    std::cout << "Next stop: " << leg.stop->name << std::endl;
                    std::cout << "\tDate and time: " << leg.arrival << std::endl;
                    if (leg.transport) {
                        std::cout << "\tArrived by " << leg.transport->trip->route->desc
                            << " " << leg.transport->trip->route->short_name
                            << " direction to " << leg.transport->trip->head_sign << std::endl;
                    }
                    if (leg.transfer) {
                        std::cout << "\tArrived by foot. Transfer time: " << leg.transfer->duration << std::endl;
                    }
                }
            } catch (std::exception const& e) {
                std::cout << "Something wrong: " << e.what() << std::endl;
            }
        }
    } catch (std::exception const& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
}