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
#include "http_request.hpp"
#include <boost/bind.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <gumbo.h>
//#include <signal.h>

Crawler::Crawler()
  : logger("Crawler"),
    signals(io_service),
    db("test.db")
{
  //signals.add(SIGINT);
  //signals.add(SIGTERM);
  //signals.add(SIGQUIT);
  //signals.async_wait(bind(&Crawler::handle_stop, this));
}
 
Crawler::~Crawler()
{
}
 
void Crawler::start()
{
  // Reasonably solid for single request at a time
  // Multiple concurrent requests is not goal
  
  //sqlite db("test.db");

  auto links = db.get_links(500);
  for(auto &link : links)
  {
    std::unique_ptr<http_request> r(new http_request(
      std::get<0>(link), std::get<1>(link) ) );
      
    r->set_protocol( std::get<2>(link) );
    r->set_request_type(RequestType::HEAD);
    request_queue.push_back(std::move(r));
  }
  
  while(!request_queue.empty())
  {
    http_request *r = request_queue.front().get();
    process_robots(r->get_server(), r->get_protocol(), db);
    
    if( check_if_header_text_html(*r) )
      {
      http_client c(io_service, *r);
      io_service.run();
      if(!r->get_data().empty())
      {
        db.set_visited(r->get_server(), r->get_path(), r->get_protocol(),
          r->get_status_code());
        db.add_links(r->get_links());
      } else {
        std::cout << "No data?\n";
        db.set_visited(r->get_server(), r->get_path(), r->get_protocol(),
          r->get_status_code());
        request_queue.pop_front();
      }
    }
    if(r->should_blacklist())
    {
      db.blacklist(r->get_server(), r->get_path(), r->get_protocol(), r->get_blacklist_reason());
    }
    
    request_queue.pop_front();
    io_service.reset();
  }
  return;
}

bool Crawler::check_if_header_text_html(http_request &request)
{
  http_client c(io_service, request);
  io_service.run();
  io_service.reset();
  auto headers = request.get_headers();
  for(auto &header : headers)
  {
    std::string lower_header = header;
    boost::to_lower(lower_header);
    std::size_t found;
    if( (found = lower_header.find("content-type: text/html") ) != std::string::npos)
    {
      request.set_request_type(RequestType::GET);
      logger.warn("IS HTML");
      return true;
    }
  }
  logger.warn("!!!!!!!!!!!!!!!!!!!!    NOT HTML  !!!!!!!!!!!!!!!!!!!!!!!!!");
  request.should_blacklist(true, "not html");
  return false;
  
  //http_clientc2(io_service, request);
  //io_service.run();
  //io_service.reset();
  //if(check_if_html(request))
  //  return true;
  //else
  //  return false;
}

bool Crawler::check_if_html(http_request &request)
{
  if( request.get_status_code() != 200 || request.get_status_code() == 0)
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

void Crawler::process_robots(std::string domain, std::string protocol, sqlite &db)
{
  // Follow SOME robots.txt rules...
  // Not fully compliant
  // This is very much a WIP
  
  if(!db.should_process_robots(domain, protocol))
    return;

  v_links blacklist;

  http_request r(domain, "/robots.txt");
  http_client c(io_service, r);
  std::string line;
  bool foundUserAgent = false;
  std::size_t found;
  io_service.run();
  std::istringstream ss(r.get_data());
  while(!ss.eof())
  {
    getline(ss, line);
    boost::to_lower(line);
    if( (found = line.find("#")) != std::string::npos )
      line = line.substr(0, found);
      
    if( (found = line.find("user-agent: ")) != std::string::npos)
    {
      if( (found = line.find("user-agent: *")) != std::string::npos)
      {
        foundUserAgent = true;
      } else {
        foundUserAgent = false;
      }
    }
    
    if( foundUserAgent && (found = line.find("disallow: ")) != std::string::npos)
    {
      std::string disallow = line.substr(10, line.length());
      std::tuple<std::string,std::string,std::string> link(domain, disallow, r.get_protocol());
      blacklist.push_back(link);
    }
  }
  if(!blacklist.empty());
    db.blacklist(blacklist, "robots.txt");
  db.set_robot_processed(domain, protocol);
  io_service.reset();
}

void Crawler::handle_stop()
{
  std::cerr << "\nCaught signal\n";
  db.close_db();
  exit(0);
}
