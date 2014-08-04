/*
 * WebCrawler: http_request.cxx
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
 * @file http_request.cxx
 * @author Kyle Givler
 */

#include "request_reciver.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <cstdlib>



http_request::http_request(
  request_reciver &reciver, 
  std::string server,
  std::string path,
  std::string protocol)
  : server(server),
    path(path),
    protocol(protocol),
    reciver(&reciver),
    logger("http_request")
{
  logger.setIgnoreLevel(Level::TRACE);
  if(this->protocol == "https" && this->port == 80)
    set_port(443);
}

http_request::~http_request() {}

void http_request::call_request_reciver(http_request *r) 
{ 
  reciver->receive_http_request(r); 
}


std::vector<std::string> http_request::get_links()
{
  std::vector<std::string> links;
  
  if(get_data().size() == 0)
    return links;
  
  GumboOutput *output = gumbo_parse(get_data().c_str());
  search_for_links(output->root, links);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
  
  return links;
}

void http_request::search_for_links(GumboNode *node, std::vector<std::string> &links)
{
  if(node->type != GUMBO_NODE_ELEMENT)
    return;
  
  GumboAttribute *href;
  if(node->v.element.tag == GUMBO_TAG_A &&
    (href = gumbo_get_attribute(&node->v.element.attributes, "href")))
  { // Minimal normilization
    std::size_t found = std::string::npos;
    std::string link = href->value;
    
    link = boost::algorithm::replace_all_copy(link, "..", "");
    
    if(link[0] == '/' && link[1] == '/')
    {
      logger.trace("Fixing link: " + link);
      link.erase(0,2);
      link.insert(0, "http://");
    }
    
    if(link[0] == '/')
    {
      logger.trace("Fixing link: " + link);
      link.insert(0, get_server());
    }

    if(link.find(get_server()) == std::string::npos && link.find("://") == std::string::npos)
    {
      logger.trace("Fixing link: " + link);
      link.insert(0, get_server() + "/");
    }

    found = link.find("#");
    if(found != std::string::npos)
    {
      logger.trace("Dropping link: " + link);
      return;
    }

    found = link.find("?");
    if(found != std::string::npos)
    {
      logger.trace("Dropping link: " + link);
      return;
    }

    if( (found = link.find("javascript:")) != std::string::npos)
    {
      logger.debug("Dropping link: "+ link);
      return;
    }

    found = link.find("%");
    if(found != std::string::npos)
    {
      if(!isdigit(link[found + 1]))
      {
        link[found + 1] = toupper(link[found + 1]);
        logger.debug("Normalilzed: " + link);
      }
      if(!isdigit(link[found + 2]))
      {
        link[found + 2] = toupper(link[found + 1]);
        logger.debug("Normalilzed: " + link);
      }
    }

    found = link.find(":80");
    if(found != std::string::npos)
    {
      std::string before = link.substr(0, found);
      std::string after = link.substr(found + 3, link.length());
      link = before + after;
      logger.debug("Normailzed: " + link);
    }
    
    logger.trace("Adding link: " + link);
    links.push_back(link);
  }
  
  GumboVector *children = &node->v.element.children;
  for(std::size_t i = 0; i < children->length; ++i)
    search_for_links(static_cast<GumboNode*>(children->data[i]), links);
}
