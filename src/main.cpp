#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "http_client.hpp"
#include "http_request.hpp"
#include <gumbo.h>
#include <cassert>

int main(int argc, char **argv)
{
  asio::io_service io_service;

  boost::thread_group threads; 
   
  http_client hc(io_service);
  //http_request rq("www.google.com", "/");
  http_request rq2("reddit.com", "/");
  //rq2.set_request_type(RequestType::GET);
  //hc.make_request(rq);
  hc.make_request(rq2);
  
  for(std::size_t i = 0; i < 2; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  io_service.run();
  threads.join_all();
  
  std::cout << "\n";
  auto headers = rq2.get_headers();
  for(auto &header : headers)
    std::cout << header;
  std::cout << "\n";
  
  //std::cout << rq.get_data() << std::endl;
  
  //auto links = rq2.get_links();
  //for(auto &link : links)
  //  std::cout << link << "\n";
  
  return 0;
}
