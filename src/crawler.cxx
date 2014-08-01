/*
 * WebCrawler: crawler.cxx
 * Copyright (C) 2014 Kyle Givler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file crawler.cxx
 * @author Kyle Givler
 */
 
#include "crawler.hpp"
#include "http_client.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <gumbo.h>
#include <csignal>

Crawler::Crawler()
  : logger("Crawler"),
    signals(io_service),
    db("test.db"),
    strand(io_service)
{
  signals.add(SIGINT);
  signals.add(SIGTERM);
  signals.add(SIGQUIT);
  signals.async_wait(bind(&Crawler::handle_stop, this));
}
 
Crawler::~Crawler()
{
}
 
void Crawler::receive_http_request(std::unique_ptr<http_request> r)
{
  std::cout << "!!!!!!!!!!!!!!!! I GOT CALLED !!!!!!!!!!!!!!!!!!!!\n";
  auto headers = r->get_headers();
  for(auto &header : headers)
    std::cout << header;

  io_service.stop();
}
 
void Crawler::start()
{
  using namespace std;
  http_client c(io_service);
  std::unique_ptr<http_request> r(new http_request(*this, "www.google.com", "/"));
  c.make_request(std::move(r));
  io_service.run();
  
  // Reasonably solid for single request at a time
  // Multiple concurrent requests is next goal

  //asio::io_service::work work(io_service);
  //std::thread t(bind(&asio::io_service::run, &io_service));

  //auto links = db.get_links(5);
  //for(auto &link : links)
  //{
  //  request_queue.push_back(link);
  //}

}


bool Crawler::check_if_header_text_html(http_request &request)
{
  
  auto headers = request.get_headers();
  for(auto &header : headers)
  {
    std::string lower_header = header;
    boost::to_lower(lower_header);
    std::size_t found;
    if( (found = lower_header.find("content-type: text/html") ) != std::string::npos)
    {
      logger.warn("IS HTML");
      return true;
    }
  }
  logger.warn("!!!!!!!!!!!!!!!!!!!!    NOT HTML  !!!!!!!!!!!!!!!!!!!!!!!!!");
  request.should_blacklist(true, "not html");
  return false;
}

bool Crawler::check_if_html(http_request &request)
{
  // This method currently isn't used anywhere
  
  
  if( request.get_status_code() != 200)
    return false;
  
  bool is_html = false;
  GumboOutput *output = gumbo_parse(request.get_data().c_str());
  GumboNode *node = output->root;
  
  if(node->v.element.tag == GUMBO_TAG_HTML)
   is_html = true;
  
  if(!is_html)
  { // Pretty sure this should never be needed, but just in case
    while(node->type == GUMBO_NODE_ELEMENT)
    {
      GumboVector *children = &node->v.element.children;
      for(std::size_t i = 0; i < children->length; i++)
        if(static_cast<GumboNode*>(children->data[i])->v.element.tag == GUMBO_TAG_HTML)
        {
          logger.warn("I guess it was needed!");
          is_html = true;
          break;
        }
    }
  }
  
  gumbo_destroy_output(&kGumboDefaultOptions, output);
  return is_html;
}

//void Crawler::process_robots(std::string domain, std::string protocol, sqlite &db)
//{
  //// Follow SOME robots.txt rules...
  //// Not fully compliant
  
  //if(!db.should_process_robots(domain, protocol))
    //return;

  //v_links blacklist;

  ////http_request r(domain, "/robots.txt");
  ////http_client c(io_service);
  ////c.make_request(r);
  //std::string line;
  //bool foundUserAgent = false;
  //std::size_t found;
  //io_service.run();
  ////std::istringstream ss(r.get_data());
  //std::istringstream ss("");
  //while(!ss.eof())
  //{
    //getline(ss, line);
    //boost::to_lower(line);
    //if( (found = line.find("#")) != std::string::npos )
      //line = line.substr(0, found);
      
    //if( (found = line.find("user-agent: ")) != std::string::npos)
    //{
      //if( (found = line.find("user-agent: *")) != std::string::npos)
      //{
        //foundUserAgent = true;
      //} else {
        //foundUserAgent = false;
      //}
    //}
    
    //if( foundUserAgent && (found = line.find("disallow: ")) != std::string::npos)
    //{
      //std::string disallow = line.substr(10, line.length());
      //std::tuple<std::string,std::string,std::string> link(domain, disallow, r.get_protocol());
      //blacklist.push_back(link);
    //}
  //}
  //if(!blacklist.empty());
    //db.blacklist(blacklist, "robots.txt");
  //db.set_robot_processed(domain, protocol);
  //io_service.reset();
//}

void Crawler::seed(std::string domain, std::string path)
{
  db.add_link(domain + path);
}

void Crawler::handle_stop()
{
  std::cerr << "\nCaught signal\n";
  db.close_db();
  exit(0);
}
