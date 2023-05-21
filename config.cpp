#include <iostream>

#include "config.h"

void load_config_group(KafkaConfigs& kafka_configs, GKeyFile *key_file, const char *group) 
{
  g_autoptr(GError) error = NULL;

  gchar **ptr = g_key_file_get_keys(key_file, group, NULL, &error);

  if (error) {
    g_error("%s", error->message);
    exit(1);
  }

  while (*ptr) {
    const char *key = *ptr;
    g_autofree gchar *value = g_key_file_get_string(key_file, group, key, &error);

    if (error) {
      g_error("Reading key: %s", error->message);
      exit(1);
    }

    kafka_configs.insert(std::make_pair<std::string, std::string>(key, value));

    ptr++;
  }
}

void set_config(
  RdKafka::Conf* kafka_config, 
  const std::string& config_file,
  RdKafka::DeliveryReportCb& dr_callback
) {
  g_autoptr(GError) error = NULL;
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  if (!g_key_file_load_from_file (key_file, config_file.c_str(), G_KEY_FILE_NONE, &error)) {
      g_error ("error loading config file: %s", error->message);
  }

  KafkaConfigs kafka_configs;
  load_config_group(kafka_configs, key_file, "default");
  load_config_group(kafka_configs, key_file, "producer");

  std::string errstr;
  for (auto& config : kafka_configs) {
    if (kafka_config->set(config.first, config.second, errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << errstr << std::endl;
        exit(1);
    }
  }

  if (kafka_config->set("dr_cb", &dr_callback, errstr) != RdKafka::Conf::CONF_OK) {
    std::cerr << errstr << std::endl;
    exit(1);
  }
}
