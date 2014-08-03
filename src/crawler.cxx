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
#include <gumbo.h>
#include <iostream>
#include <csignal>

Crawler::Crawler(boost::asio::io_service &io_service)
  : client(io_service),
    signals(io_service),
    strand(io_service),
    io_service(io_service),
    db(new sqlite("test.db")),
    logger("Crawler")
{
  logger.setIgnoreLevel(Level::NONE);
  
  signals.add(SIGINT);
  signals.add(SIGTERM);
  #ifdef SIGQUIT
  signals.add(SIGQUIT);
  #endif
  signals.async_wait(bind(&Crawler::handle_stop, this));
}
 
Crawler::~Crawler()
{
  db->close_db();
  delete(db);
}
 
void Crawler::receive_http_request(http_request *r)
{
  logger.trace("Recived completed request: " + r->get_protocol() + "://"
    + r->get_server() + r->get_path() );
    
  std::size_t found;
  if( (found = r->get_path().find("/robots.txt") == 0) )
  {
    handle_recived_robots(r);
    return;
  }
  
  if (r->get_request_type() == RequestType::HEAD)
  {
    handle_recived_head(r);
    return;
  }
  
  if(r->get_request_type() == RequestType::GET)
  {
    handle_recived_get(r);
    return;
  }
  
  logger.severe("Fell out of request reciver!!!!!!!!!!!!!!!!!");
}


void Crawler::handle_recived_robots(http_request *request)
{
  process_robots(request->get_server(), request->get_protocol(),
    request->get_data());
  
  logger.trace("handle_recived_robots: deleting pointer");
  delete(request);
  pDeleted++;
  
  return;
}
  
void Crawler::handle_recived_head(http_request *r)
{
  if(check_if_header_text_html(r->get_headers()))
  {
    logger.trace("Converting to get request");
    r->set_request_type(RequestType::GET);
    strand.post(bind(&Crawler::do_request, this, r));
  } else {
    db->blacklist(r->get_server(), r->get_path(), r->get_protocol(),
      "Probably not html");
      
    logger.trace("Deleting pointer becasue not HTML");
    delete(r);
    pDeleted++;
    prepare_next_request();
  }
  return;
}
  
void Crawler::handle_recived_get(http_request *r)
{
  if(r->should_blacklist())
    db->blacklist(r->get_server(), r->get_path(), r->get_protocol(),
      r->get_blacklist_reason());

  if(r->get_data().size() != 0)
    db->add_links(r->get_links());
  
  if(!r->get_timed_out())
  {
    db->set_visited(r->get_server(), r->get_path(), r->get_protocol(),
      r->get_status_code());
  }
    
  logger.trace("Get: Deleting request, no longer needed");
  delete(r);
  pDeleted++;
  
  prepare_next_request();
  
  return;
}

void Crawler::prepare_next_request()
{
  logger.trace("Preparing next request: Queue size: " +
    std::to_string(request_queue.size()));
  
  logger.trace("Pointers: created: " + std::to_string(pCreated) + 
    " deleted: " + std::to_string(pDeleted));
  
  if(!request_queue.empty())
  {
    auto t_request = request_queue.front();
    std::string domain = std::get<0>(t_request);
    std::string path = std::get<1>(t_request);
    std::string protocol = std::get<2>(t_request);
    
    logger.trace("Creating pointer with: " + domain + " " + path + " "
      + protocol);
      
    http_request *request = new http_request(*this, domain, path,
      protocol);
    pCreated++;
    request->set_request_type(RequestType::HEAD);
    
    if(db->should_process_robots(domain, protocol))
    {
      request->set_path("/robots.txt");
      request->set_request_type(RequestType::GET);
    } else {
      request_queue.pop_front();
    }
    strand.post(bind(&Crawler::do_request, this, request));
    
  } else {
    std::cout << "Queue is empty, quiting\n";
    db->close_db();
    exit(0);
  }
}

void Crawler::do_request(http_request *r)
{
  logger.trace("Sending request to client: " + r->get_protocol() + "://"
    + r->get_server() + r->get_path());
  strand.post(bind(&http_client::make_request, &client, r));
  return;
}

void Crawler::start()
{
  auto links = db->get_links(100);
  for(auto &link : links)
    request_queue.push_back(link);

  prepare_next_request();
  //strand.post(bind(&Crawler::prepare_next_request, this, 
  //  RequestType::HEAD));
}


bool Crawler::check_if_header_text_html(std::vector<std::string> headers)
{
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
  logger.info("Probably is NOT HTML");
  return false;
}

void Crawler::process_robots(
  std::string server,
  std::string protocol,
  std::string data)
{
  // Follow SOME robots.txt rules...
  // Not fully compliant
  logger.debug("Proccessing robots.txt for: " + protocol + "://" + 
    server);
  
  v_links blacklist;
  std::string line;
  bool foundUserAgent = false;
  std::size_t found;
  
  std::istringstream ss(data);
  while(!ss.eof())
  {
    getline(ss, line);
    boost::to_lower(line);
    if( (found = line.find("#")) != std::string::npos )
      line = line.substr(0, found);
      
    if( (found = line.find("user-agent: ")) != std::string::npos)
    {
      if( (found = line.find("user-agent: *")) != std::string::npos ||
           (found = line.find("user-agent: joyfulreaper") != std::string::npos) )
      {
        foundUserAgent = true;
      } else {
        foundUserAgent = false;
      }
    }
    
    if( foundUserAgent && (found = line.find("disallow: ")) != std::string::npos)
    {
      std::string disallow = line.substr(10, line.length());
      std::tuple<std::string,std::string,std::string> link(server, 
        disallow, protocol);
      blacklist.push_back(link);
    }
  }
  if(!blacklist.empty());
    db->blacklist(blacklist, "robots.txt");
  db->set_robot_processed(server, protocol);
  
  prepare_next_request();
}

void Crawler::seed(std::string domain, std::string path)
{
  db->add_link(domain + path);
  std::cout << "Added seed to database\n";
  exit(0);
}

void Crawler::handle_stop()
{
  std::cerr << "\nCaught signal\n";
  io_service.stop();
  db->close_db();
  exit(0);
}

bool Crawler::check_if_html(std::string data)
{
  // This method currently isn't used anywhere
  
  bool is_html = false;
  GumboOutput *output = gumbo_parse(data.c_str());
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
