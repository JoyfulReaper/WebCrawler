/*
 * WebCrawler: robot_parser.cxx
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
 * @file robot_parser.cxx
 * @author Kyle Givler
 */

#include "robot_parser.hpp"
#include "database.hpp"
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>

using namespace boost;

robot_parser::robot_parser() : logger("robot_parser")
{
}

bool robot_parser::path_is_allowed(std::string i_pattern, std::string path)
{
  std::string pattern = algorithm::replace_all_copy(i_pattern, "*", ".*");
  pattern = algorithm::replace_all_copy(pattern, "?", "\\?");
  pattern = algorithm::replace_all_copy(pattern, ".", "\\.");
  
  regex exp(pattern);
  cmatch what;
  
  if(regex_match(path.c_str(), what, exp))
    return false; // Path not allowed

  return true; // Path is allowed 
}

void robot_parser::process_robots(
  std::string server,
  std::string protocol,
  std::string data,
  database *db)
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
  
  return;
}
