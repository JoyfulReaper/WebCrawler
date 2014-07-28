/*
 * WebCrawler: sqlite_database.hpp
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
 * @file sqlite_database.hpp
 * @author Kyle Givler
 */

#ifndef _SQLITE_DATABASE_H_
#define _SQLITE_DATABASE_H_

#include <sqlite3.h>
#include <string>
#include <memory>
#include "http_request.hpp"
#include "logger/logger.hpp"

using namespace boost;
typedef std::shared_ptr<http_request> s_request;

class sqlite_database
{
public:
  sqlite_database(std::string databaseFile);

  virtual ~sqlite_database();
  
  void add_links(s_request request);
  
  void set_visited(s_request request);
  
  bool get_visited(s_request request);
  
  
  
private:
  std::string databaseFile;
  sqlite3 *db;
  Logger logger;
};

#endif
