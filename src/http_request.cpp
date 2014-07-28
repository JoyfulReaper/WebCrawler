/*
 * WebCrawler: http_request.cpp
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


#include "http_request.hpp"
#include <boost/algorithm/string/case_conv.hpp>
#include <cstdlib>

static const bool DEBUG = true;

http_request::http_request(std::string server, std::string path, unsigned int port)
  : port(port),
    server(server),
    path(path),
    logger("http_request")
{
  logger.setIgnoreLevel(Level::TRACE);
}

http_request::~http_request() {}

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
    if(link[0] == '/')
      link.insert(0, get_server());

    // This one is iffy :(
    if(link.find(get_server()) == std::string::npos && link.find("://") == std::string::npos)
    {
      link.insert(0, get_server() + "/");
      if(DEBUG)
        logger.debug("Fixing link: " + link);
    }

    boost::to_lower(link);

    found = link.find("#");
    if(found != std::string::npos)
    {
      if(DEBUG)
        logger.debug("Dropped link: " + link);
      return;
    }

    found = link.find("?");
    if(found != std::string::npos)
    {
      if(DEBUG)
        logger.debug("Dropped link: " + link);
      return;
    }

    found = link.find("%");
    if(found != std::string::npos)
    {
      if(!isdigit(link[found + 1]))
      {
        link[found + 1] = toupper(link[found + 1]);
        if(DEBUG)
          logger.debug("Normalilzed: " + link);
      }
      if(!isdigit(link[found + 2]))
      {
        link[found + 2] = toupper(link[found + 1]);
        if(DEBUG)
          logger.debug("Normalilzed: " + link);
      }
    }

    found = link.find(":80");
    if(found != std::string::npos)
    {
      std::string before = link.substr(0, found);
      std::string after = link.substr(found + 3, link.length());
      link = before + after;
      if(DEBUG)
        logger.debug("Normailzed: " + link);
    }
    
    std::cout << link << std::endl;
    links.push_back(link);
  }
  
  GumboVector *children = &node->v.element.children;
  for(std::size_t i = 0; i < children->length; ++i)
    search_for_links(static_cast<GumboNode*>(children->data[i]), links);
}
