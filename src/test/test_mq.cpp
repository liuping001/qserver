/**
 *  LibBoostAsio.cpp
 *
 *  Test program to check AMQP functionality based on Boost's asio io_service.
 *
 *  @author Gavin Smith <gavin.smith@coralbay.tv>
 *
 *  Compile with g++ -std=c++14 libboostasio.cpp -o boost_test -lpthread -lboost_system -lamqpcpp
 */

/**
 *  Dependencies
 */
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "third/AMQP-CPP/include/amqpcpp.h"
#include "third/AMQP-CPP/include/amqpcpp/libboostasio.h"

/**
 *  Main program
 *  @return int
 */
int main(int argc , char **argv) {

  // access to the boost asio handler
  // note: we suggest use of 2 threads - normally one is fin (we are simply demonstrating thread safety).
  boost::asio::io_service service(2);

  // handler for libev
  AMQP::LibBoostAsioHandler handler(service);

  // make a connection
  AMQP::TcpConnection connection(&handler, AMQP::Address(argv[1]));

  // we need a channel too
  AMQP::TcpChannel channel(&connection);


  channel.declareExchange("router1", AMQP::ExchangeType::direct).onSuccess([](){
    std::cout << "declare exchange success" << std::endl;
  });


  for (auto i = 0; i <= 10; i++) {
    channel.publish("router1", "1.1.1.1", "hello " + std::to_string(i)).onError([](const char *message){
      std::cout << message << "\n";
    });
  }

  // run the handler
  // a t the moment, one will need SIGINT to stop.  In time, should add signal handling through boost API.
  return service.run();
}