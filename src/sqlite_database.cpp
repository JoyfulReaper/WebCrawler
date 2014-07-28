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
#include <boost/asio.hpp>

sqlite_database::sqlite_database(std::string databaseFile)
  : databaseFile(databaseFile),
    logger("sqlite")
{
  int rc = sqlite3_open(databaseFile.c_str(), &db);
  if(rc)
    throw("Can't open database: " + databaseFile);
    
  logger.setIgnoreLevel(Level::TRACE);
}

sqlite_database::~sqlite_database()
{
  if(db)
    sqlite3_close(db);
}

void sqlite_database::add_links(s_request request)
{
  std::string protocol;
  std::string domain;
  std::string path;
  
  while(!request->get_completed())
    sleep(1);
  
  auto links = request->get_links();
  for(auto &link : links)
  {
    std::size_t found = link.find("://");
    if(found != std::string::npos)
    {
      protocol = link.substr(0, found);
      link = link.substr(found + 3, link.length());
    }
    else
      protocol = "http";
      
    if(protocol != "http")
    {
      logger.debug("sqlite: Dropping: " + link);
      break;
    }
    
    found = link.find("/");
    if(found != std::string::npos)
    {
      domain = link.substr(0, found);
      path = link.substr(found, link.length());
    } else {
      domain = link;
      path = "/";
    }
    
    std::string sql = "INSERT INTO Links (domain,path,protocol) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "');";
   
   logger.trace("SQL: " + sql);
   
   char *err = 0;
   int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
   
   if(rc != SQLITE_OK)
   {
      if(rc != SQLITE_CONSTRAINT) // constraint failed
      {
        logger.debug("ERROR CODE: " + std::to_string(rc));
        throw("SQLITE ERROR: add_links");
      }
   }
   
  }
  return;
}

bool sqlite_database::get_visited(s_request request)
{
  sqlite3_stmt *statement;
  int rc;
  
  std::string sql = "SELECT visited FROM Links WHERE domain = '" \
    + request->get_server() + "' AND path = '" + request->get_path() + \
    "' AND protocol = '" + request->get_protocol() + "';";

  rc= sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    logger.debug("ERROR CODE: " + std::to_string(rc));
    throw("SQLITE ERROR");
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_DONE)
  {
    logger.debug("ERROR CODE: " + std::to_string(rc));
    throw("SQLITE ERROR: get_visited");
  }
  
  return sqlite3_column_int(statement, 0);
}

void sqlite_database::set_visited(s_request request)
{
  sqlite3_stmt *statement;
  int rc;
  
  std::string sql = "UPDATE Links SET visited = '1' WHERE domain = '" \
    + request->get_server() + "' AND path = '" + request->get_path() + \
    "' AND protocol = '" + request->get_protocol() + "';";
  
  std::cout << sql;
  
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    logger.debug("ERROR CODE: " + std::to_string(rc));
    throw("SQLITE ERROR");
  }
  
  rc = sqlite3_step(statement);
  
  if(rc != SQLITE_DONE)
  {
    logger.debug("ERROR CODE: " + std::to_string(rc));
    throw("SQLITE ERROR: set_visited");
  }
}
