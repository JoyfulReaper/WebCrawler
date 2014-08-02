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
#include <boost/bind.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <gumbo.h>
#include <csignal>

Crawler::Crawler()
  : client(io_service),
    signals(io_service),
    strand(io_service),
    db("test.db"),
    logger("Crawler")
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
  io_service.reset();
  
  logger.info("Queue size: " + std::to_string(request_queue.size()));
  
  if(r->get_path() == "/robots.txt")
  {
    process_robots(*r);
    prepare_next_request();
  }
    
  if(r->get_request_type() == RequestType::HEAD)
  {
    if (check_if_header_text_html(*r))
    {
      r->set_request_type(RequestType::GET);
      do_request(std::move(r));
    }
  }
  
  if(r->should_blacklist())
    db.blacklist(r->get_server(), r->get_path(), r->get_protocol(), 
      r->get_blacklist_reason());
    
  db.add_links(r->get_links());
  db.set_visited(r->get_server(), r->get_path(), r->get_protocol(), 
    r->get_status_code());
  
  
  request_queue.pop_front();
  
  prepare_next_request();
  
}

void Crawler::prepare_next_request()
{
  if(!request_queue.empty())
  {
    auto r = request_queue.front();
    std::string server = std::get<0>(r);
    std::string path = std::get<1>(r);
    std::string protocol = std::get<2>(r);
    
    if(protocol != "http" && protocol != "https")
      protocol = "http";
    
    std::unique_ptr<http_request> request(
      new http_request(*this, server, path, protocol));
    request->set_request_type(RequestType::HEAD);
    
    if(db.should_process_robots(server, protocol) )
    {
      request->set_path("/robots.txt");
      request->set_request_type(RequestType::GET);
    }
    
    do_request(std::move(request));
    
  } else {
    std::cerr << "Empty queue\n";
    exit(0);
  }
}

void Crawler::do_request(std::unique_ptr<http_request> request)
{
  client.make_request(std::move(request));
  io_service.run();
}

void Crawler::start()
{
  //asio::io_service::work work(io_service);
  //std::thread t(bind(&asio::io_service::run, &io_service));

  http_client client(io_service);

  auto links = db.get_links(1000);
  for(auto &link : links)
    request_queue.push_back(link);

  prepare_next_request();
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
      logger.warn("Probably is HTML");
      return true;
    }
  }
  logger.warn("!!!!!!!!!!!!!!!!!!!!  Probably Not HTML  !!!!!!!!!!!!!!!!!!!!!!!!!");
  request.should_blacklist(true, "probably not html");
  return false;
}

void Crawler::process_robots(http_request &request)
{
  // Follow SOME robots.txt rules...
  // Not fully compliant

  v_links blacklist;
  std::string line;
  bool foundUserAgent = false;
  std::size_t found;
  
  std::istringstream ss(request.get_data());
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
      std::tuple<std::string,std::string,std::string> link(request.get_server(), 
        disallow, request.get_protocol());
      blacklist.push_back(link);
    }
  }
  if(!blacklist.empty());
    db.blacklist(blacklist, "robots.txt");
  db.set_robot_processed(request.get_server(), request.get_protocol());
}

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

bool Crawler::check_if_html(http_request &request)
{
  // This method currently isn't used anywhere
  
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
