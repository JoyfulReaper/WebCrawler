/*
 * WebCrawler: robot_parser.hpp
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
 * @file robot_parser.hpp
 * @author Kyle Givler
 */

#ifndef _WC_ROBOT_PARSER_H_
#define _WC_ROBOT_PARSER_H_

#include <string>
#include "logger/logger.hpp"

class database;

class robot_parser
{
public:
  robot_parser();

  bool path_is_allowed(
  std::string pattern, 
  std::string path);
  
void process_robots(
  std::string server, 
  std::string protocol, 
  std::string data, database *db);
  
private:
  Logger logger;
};

#endif
