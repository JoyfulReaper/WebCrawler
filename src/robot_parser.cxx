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
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace boost;

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
