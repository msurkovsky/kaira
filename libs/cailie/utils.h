
#ifndef CAILIE_UTILS_H
#define CAILIE_UTILS_H

#include <string>

/* parse_size_string("100") == 100
 * parse_size_string("20M") == 20 * 1024 * 1024
 * parse_size_string("ABC") == 0
 * support suffixes: K, M, G
 */
size_t ca_parse_size_string(const std::string &str);

#endif
