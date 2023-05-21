#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <chrono>
#include <iomanip>

#include <unistd.h>
#include <stdlib.h>

#include <librdkafka/rdkafkacpp.h>

#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include "config.h"
#include "message.h"

class DeliveryReportCallback : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb(RdKafka::Message &message) {
    if (message.err())
      std::cerr << "message delivery failed: " << message.errstr()
                << std::endl;
#ifdef DEBUG
    else
      std::cerr << "message delivered to topic " << message.topic_name()
                << " [" << message.partition() << "] at offset "
                << message.offset() << std::endl;
#endif
  }
};

boost::lockfree::queue<Message> queue(1024);

const int num_messages = 2500;
const std::string topic = "topic_001";
const std::string config_file = "kafka_config.ini";

boost::atomic<bool> done(false);
boost::atomic_int producer_count(0);

void producer()
{
  for (int i = 0; i < num_messages; ++i)
  {
    Message message = create_message();
    ++producer_count;
    while(!queue.push(message));
    // throttle production
    boost::this_thread::sleep_for(boost::chrono::milliseconds((rand() % 100) + 10));
  }
}

void consumer()
{
  // create a kafka producer config
  RdKafka::Conf* config = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  DeliveryReportCallback dr_callback;
  set_config(config, config_file, dr_callback);

  // create kafka producer from config
  std::string errstr;
  RdKafka::Producer* producer = RdKafka::Producer::create(config, errstr);
  if (!producer) {
    std::cerr << "failed to create producer: " << errstr << std::endl;
    exit(1);
  }

  Message message;
  while(!done)
  {
    while(queue.pop(message))
    {
      // convert message to string
      std::string message_str = message.to_string();

      // send message string to kafka cluster
      RdKafka::ErrorCode err = producer->produce(
        topic,
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char *>(message_str.c_str()), message_str.size(),
        NULL, 
        0,
        0,
        NULL,
        NULL
      );

      if (err != RdKafka::ERR_NO_ERROR) {
        if (err == RdKafka::ERR__QUEUE_FULL) {
          producer->poll(1000 /*block for max 1000ms*/);
        }
      }

      producer->poll(0);
    }
  }

  std::cerr << "flushing final messages..." << std::endl;
  producer->flush(10 * 1000 /* wait for max 10 seconds */);

  if (producer->outq_len() > 0)
    std::cerr << producer->outq_len() << " message(s) were not delivered" << std::endl;

  delete producer;
}

int main(int argc, char **argv) {

  // arguments : 
  //  - p : nr. of producers
  //  - c : nr. of consumers (and Kafka producers)
  if (argc != 3) {
    std::cerr << "usage: " << argv[0] << " <nr. of producers> <nr. of consumers>\n";
    exit(1);
  }

  const int producer_thread_count = atoi(argv[1]);
  const int consumer_thread_count = atoi(argv[2]);

  // is the queue lockfree or not?
  std::cout << "boost::lockfree::queue is ";
  if (!queue.is_lock_free()) {
    std::cout << "not ";
  }
  std::cout << "lockfree" << std::endl;

  // create and start consumer and producer threads
  boost::thread_group producer_threads, consumer_threads;
  auto startTime = std::chrono::high_resolution_clock::now();

  for (int i = 0; i != consumer_thread_count; ++i)
      consumer_threads.create_thread(consumer);
  for (int i = 0; i != producer_thread_count; ++i)
      producer_threads.create_thread(producer);

  // wait for producer threads to finish
  producer_threads.join_all();

  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = endTime - startTime;
  std::cout << std::fixed 
    << std::setprecision(3)
    << "producers stopped after " 
    << elapsed.count() << " seconds (" 
    << (static_cast<double>(producer_count) / elapsed.count()) << " msg/s)" << std::endl;

  // wait 5 seconds before signaling consumer threads
  sleep(5);

  // signal consumer threads and wait for them to finish
  done = true;
  consumer_threads.join_all();

  std::cout << "done" << std::endl;

  return 0;
}
