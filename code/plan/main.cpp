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
        util::parse(feed_directory);
    } catch (std::exception const& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Success!" << std::endl;
    return 0;
}