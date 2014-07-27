#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "http_client.hpp"
#include "http_request.hpp"
#include <tidy.h>
#include <buffio.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
 
 
struct html_test
{
  std::string title;
  void load(std::istringstream &stream)
  {
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(stream, pt);
    
    title = pt.get("html.head.title", "null");
    return;
  }
};
 
std::string CleanHTML(const std::string &html){
  // Initialize a Tidy document
  TidyDoc tidyDoc = tidyCreate();
  TidyBuffer tidyOutputBuffer = {0};

  // Configure Tidy
  // The flags tell Tidy to output XML and disable showing warnings
  bool configSuccess = tidyOptSetBool(tidyDoc, TidyXmlOut, yes)
      && tidyOptSetBool(tidyDoc, TidyQuiet, yes)
      && tidyOptSetBool(tidyDoc, TidyNumEntities, yes)
      && tidyOptSetBool(tidyDoc, TidyShowWarnings, no);

  int tidyResponseCode = -1;

  // Parse input
  if (configSuccess)
      tidyResponseCode = tidyParseString(tidyDoc, html.c_str());

  // Process HTML
  if (tidyResponseCode >= 0)
      tidyResponseCode = tidyCleanAndRepair(tidyDoc);

  // Output the HTML to our buffer
  if (tidyResponseCode >= 0)
      tidyResponseCode = tidySaveBuffer(tidyDoc, &tidyOutputBuffer);

  // Any errors from Tidy?
  if (tidyResponseCode < 0)
      throw ("Tidy encountered an error while parsing an HTML response. Tidy response code: " + tidyResponseCode);

  // Grab the result from the buffer and then free Tidy's memory
  std::string tidyResult = (char*)tidyOutputBuffer.bp;
  tidyBufFree(&tidyOutputBuffer);
  tidyRelease(tidyDoc);

  return tidyResult;
}


int main(int argc, char **argv)
{
  asio::io_service io_service;

  boost::thread_group threads; 
   
  http_client hc(io_service);
  http_request rq("www.google.com", "/");
  http_request rq2("reddit.com");
  rq.set_request_type(RequestType::GET);
  hc.make_request(rq);
  //hc.make_request(rq2);
  
  for(std::size_t i = 0; i < 2; i++)
    threads.create_thread(boost::bind(&asio::io_service::run, &io_service));
  
  io_service.run();
  threads.join_all();
  
  std::cout << "\n";
  auto headers = rq.get_headers();
  for(auto &header : headers)
    std::cout << header;
  std::cout << "\n";
    
  std::cout << "\n";
  headers = rq2.get_headers();
  for(auto &header : headers)
    std::cout << header;
  std::cout << "\n";
  
  //std::cout << rq.get_data() << std::endl;
  std::cout << CleanHTML(rq.get_data().c_str());
  
  std::istringstream ss;
  ss.str(CleanHTML(rq.get_data().c_str()));
  html_test test;
  test.load(ss);
  std::cout << "TITLE: " << test.title << std::endl;
  
  return 0;
}
