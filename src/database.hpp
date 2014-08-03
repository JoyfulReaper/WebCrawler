/*
 * WebCrawler: database.hpp
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
 * @file database.hpp
 * @author Kyle Givler
 */

#ifndef _WC_DATABASE_H_
#define _WC_DATABASE_H_

#include <vector>
#include <tuple>

typedef std::vector<std::tuple<std::string,std::string,std::string>> v_links;

class database
{
public:
  virtual ~database() {}

  /**
   * Close the database
   */
  virtual void close_db() = 0;
  
  /**
   * @param links A vector of links to add to the database
   */
  virtual void add_links(std::vector<std::string> links) = 0;
  
  /**
   * @param link A single link to add to the database
   */
  virtual void add_link(std::string link) = 0;
  
  /**
   * @return true if the URL has been visited, false if not
   */
  virtual bool get_visited(
    std::string domain, 
    std::string path, 
    std::string protocol) = 0;
  
  /**
   * Set resource as visited
   */
  virtual void set_visited(
    std::string domain,
    std::string path, 
    std::string protocol,
    unsigned int code) = 0;
  
  /**
   * Update the last visited date
   */
  virtual void set_last_visited(
    std::string domain, 
    std::string path, 
    std::string protocol) = 0;
  
  /**
   * @param num The number of links to return
   * @return a vector of tuples representing links
   */
  virtual v_links get_links(std::size_t num) = 0;
  
  virtual bool check_blacklist(
    std::string domain, 
    std::string path, 
    std::string proto) = 0;
    
  virtual void remove_link(
    std::string domain, 
    std::string path, 
    std::string protocol) = 0;
    
  virtual void blacklist(
    v_links blacklist, 
    std::string reason = "default") = 0;
  
  virtual void blacklist(
    std::string domain, 
    std::string path, 
    std::string protocol, 
    std::string reason = "default") = 0;
  
  virtual void set_robot_processed(
    std::string server, 
    std::string protocol) = 0;
  
  virtual bool should_process_robots(
    std::string domain, 
    std::string protocol) = 0;
    
private:
};

#endif
