#ifndef PLAN_PARSER_T_H
#define PLAN_PARSER_T_H

#include "structures.h"
#include "map_graph_t.h"

#include <string>

namespace util {

processing::map_graph_t parse(std::string const& feed_directory);

} // util

#endif //PLAN_PARSER_T_H
