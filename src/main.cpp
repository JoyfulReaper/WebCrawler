#include <iostream>
#include <boost/asio.hpp>
#include "http_client.hpp"
#include "http_request.hpp"

int main(int argc, char **argv)
{
  asio::io_service io_service;
  
  http_request rq("google.com");
  //http_request rq2("example.com");
  http_client hc(io_service);
  
  
  hc.make_request(rq);
  //hc.make_request(rq2);
  io_service.run();
  
  auto headers = rq.get_headers();
  for(auto &header : headers)
    std::cout << header;
    
  std::cout << "\n";
    
  //headers = rq2.get_headers();
  //for(auto &header : headers)
  //  std::cout << header;
  
  return 0;
}
