#include "option_error.h"

#include <utility>

using namespace std::string_literals;

Option_error::Option_error(std::string&& what) : what_(std::move(what)) {
}

Illegal_option::Illegal_option(int option)
    : Option_error("illegal option \'"s + static_cast<char>(option) + '\'') {
}

Missing_argument::Missing_argument(int option)
    : Option_error("option \'"s + static_cast<char>(option) +
                   "\' requires an argument") {
}

Invalid_argument::Invalid_argument(int option)
    : Option_error("option \'"s + static_cast<char>(option) +
                   "\' has an invalid argument") {
}

Invalid_argument::Invalid_argument(int option, const char* error)
    : Option_error("option \'"s + static_cast<char>(option) +
                   "\' has an invalid argument (" + error + ')') {
}
