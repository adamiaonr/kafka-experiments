#include <ctime>

struct Message {
  std::time_t timestamp;
  int value;

  std::string to_string() const;
};

Message create_message();
