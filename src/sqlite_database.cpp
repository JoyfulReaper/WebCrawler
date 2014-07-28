/*
 * WebCrawler: sqlite_database.cpp
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
 * @file sqlite_database.cpp
 * @author Kyle Givler
 */

#include "sqlite_database.hpp"

sqlite_database::sqlite_database(std::string databaseFile)
  : databaseFile(databaseFile), 
    logger("sqlite")
{
  int rc = sqlite3_open(databaseFile.c_str(), &db);
  if(rc)
    throw("Can't open database: " + databaseFile);
}

sqlite_database::~sqlite_database()
{
  if(db)
    sqlite3_close(db);
}

void sqlite_database::add_links(http_request &request)
{
  std::string protocol;
  std::string domain;
  std::string path;
  
  auto links = request.get_links();
  for(auto &link : links)
  {
    //std::cout << link << std::endl;
    std::size_t found = link.find("https://");
    if(found != std::string::npos)
      protocol = "https";
    else
      protocol = "http";
      
    found = link.find("://");
    if(found != std::string::npos)
      link = link.substr(found + 3, link.length());
    
    found = link.find("/");
    if(found != std::string::npos)
    {
      domain = link.substr(0, found);
      path = link.substr(found, link.length());
    } else {
      domain = link;
      path = "/";
    }
    
    std::string sql = "INSERT INTO LINKS (domain,path,protocol) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "');";
   
   logger.trace("SQL: " + sql);
   
   char *err = 0;
   int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
   
   if(rc != SQLITE_OK)
   {
      if(rc != SQLITE_CONSTRAINT) // constraint failed
      {
        throw("SQLITE ERROR: add_links");
      }
   }
   
  }
  return;
}
