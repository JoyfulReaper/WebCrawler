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
#include "crawlerException.hpp"
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>

sqlite_database::sqlite_database(std::string databaseFile)
  : databaseFile(databaseFile),
    logger("sqlite")
{
  int rc = sqlite3_open(databaseFile.c_str(), &db);
  if(rc)
    throw(CrawlerException("Can't open database: " + databaseFile));
    
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
      continue;
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
        std::string errmsg = "sql_add_links: ";
        errmsg.append(sqlite3_errstr(rc));
        throw(CrawlerException(errmsg));
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
    std::string errmsg = "sql_get_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_OK && rc != SQLITE_ROW)
  {
    std::string errmsg = "sql_get_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  bool ret = sqlite3_column_int(statement, 0);
  sqlite3_finalize(statement);
  
  return ret;
}

void sqlite_database::set_visited(s_request request)
{
  sqlite3_stmt *statement;
  int rc;
  
  std::string sql = "UPDATE Links SET visited = '1' WHERE domain = '" \
    + request->get_server() + "' AND path = '" + request->get_path() + \
    "' AND protocol = '" + request->get_protocol() + "';";
  
  //std::cout << sql;
  
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    std::string errmsg = "sql_set_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_DONE)
  {
    std::string errmsg = "sql_set_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  sqlite3_finalize(statement);
  set_last_visited(request);
  return;
}

void sqlite_database::set_last_visited(s_request request)
{
  using namespace std::chrono;
  system_clock::time_point tp = system_clock::now();
  system_clock::duration dtn = tp.time_since_epoch();
  
  unsigned int seconds = dtn.count() * system_clock::period::num / system_clock::period::den;
  
  sqlite3_stmt *statement;
  int rc;
  
  std::string sql = "UPDATE Links SET lastVisited = '" + std::to_string(seconds) \
    + "' WHERE domain = '" + request->get_server() + "' AND path = '" + request->get_path() + \
    "' AND protocol = '" + request->get_protocol() + "';";
  //std::cout << sql;  
  
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    std::string errmsg = "sql_set_lvist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_DONE)
  {
    std::string errmsg = "sql_set_lvist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  sqlite3_finalize(statement);
  return;
}

std::vector<s_request> sqlite_database::fill_queue()
{
  std::vector<s_request> requests;
  sqlite3_stmt *statement;
  int rc;
  
  std::string sql = "SELECT domain,path FROM Links WHERE visited = '0' LIMIT 100;";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    std::string errmsg = "sql_fill: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  while(rc != SQLITE_DONE)
  {
    if(rc == SQLITE_ROW)
    {
      std::string domain = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
      std::string path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
      
      requests.push_back(s_request (new http_request(domain, path)));
      rc = sqlite3_step(statement);
    } else {
      std::string errmsg = "sql_fill: ";
      errmsg.append(sqlite3_errstr(rc));
      throw(CrawlerException(errmsg));
    }
  }
  
  sqlite3_finalize(statement);
  return requests;
}
