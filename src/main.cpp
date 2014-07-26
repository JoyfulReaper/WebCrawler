#include <iostream>
#include <boost/asio.hpp>
#include "http_client.hpp"
#include "http_request.hpp"

int main(int argc, char **argv)
{
  asio::io_service io_service;
  
  http_request rq("google.com", 80);
  http_client hc(io_service);
  
  //rq.set_request("GET / HTTP/1.0\r\nHost google.com\r\nConnect: close\r\n\r\n");
  hc.make_request(rq);
  
  io_service.run();
  return 0;
}
