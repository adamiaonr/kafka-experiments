#include <unordered_map>
#include <string>

#include <glib.h>
#include <librdkafka/rdkafkacpp.h>

using KafkaConfigs = std::unordered_map<std::string, std::string>;

void load_config_group(KafkaConfigs& kafka_configs, GKeyFile *key_file, const char *group);
void set_config(
  RdKafka::Conf* kafka_config, 
  const std::string& config_file,
  RdKafka::DeliveryReportCb& dr_callback
);
