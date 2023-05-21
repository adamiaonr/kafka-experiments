#include <sstream>
#include <iomanip>

#include <stdlib.h>

#include "message.h"

std::string Message::to_string() const {
    std::ostringstream oss;

    oss << "{";
    oss << "\"timestamp\":\"" << std::asctime(std::localtime(&(this->timestamp))) << "\",";
    oss << "\"value\":" <<  this->value;
    oss << "}";

    return oss.str();
  }

Message create_message() {
  return { std::time(nullptr), rand() % 101 };
}
