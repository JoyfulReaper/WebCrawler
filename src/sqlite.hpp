/*
 * WebCrawler: sqlite.hpp
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
 * @file sqlite.hpp
 * @author Kyle Givler
 */

#ifndef _SQLITE_DATABASE_H_
#define _SQLITE_DATABASE_H_

#include <sqlite3.h>
#include <vector>
#include "logger/logger.hpp"

class sqlite
{
public:
  sqlite(std::string databaseFile);

  virtual ~sqlite();
  
  void add_links(std::vector<std::string> links);
  
  bool get_visited(std::string domain, std::string path, std::string protocol);
  
  void set_visited(std::string domain, std::string path, std::string protocol);

private:
  std::string databaseFile;
  sqlite3 *db;
  Logger logger;
};


#endif
