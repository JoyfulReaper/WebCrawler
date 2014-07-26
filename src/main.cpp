#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "http_client.hpp"
#include "http_request.hpp"

int main(int argc, char **argv)
{
  asio::io_service io_service;

  boost::thread_group threads; 
   
  http_client hc(io_service);
  http_request rq("google.com");
  http_request rq2("example.com");
   
  hc.make_request(rq);
  hc.make_request(rq2);
  
  for(std::size_t i = 0; i < 2; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  io_service.run();
  threads.join_all();
  
  //io_service.post(boost::bind(&http_client::make_request, &hc, rq));
  //io_service.post(boost::bind(&http_client::make_request, &hc, rq2));
  //hc.make_request(rq2);
  //io_service.stop();
  
  auto headers = rq.get_headers();
  for(auto &header : headers)
    std::cout << header;
    
  std::cout << "\n";
    
  headers = rq2.get_headers();
  for(auto &header : headers)
    std::cout << header;
  
  return 0;
}
