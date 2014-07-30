/*
 * WebCrawler: sqlite.cxx
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
 * @file sqlite.cxx
 * @author Kyle Givler
 */

#include "sqlite.hpp"
#include "crawlerException.hpp"
#include <chrono>
#include <unistd.h>
#include <boost/algorithm/string/replace.hpp>

sqlite::sqlite(std::string databaseFile)
  : databaseFile(databaseFile),
    logger("sqlite")
{
  int rc = sqlite3_open_v2(databaseFile.c_str(), &db, SQLITE_OPEN_READWRITE
    | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_close(db);
    std::string errmsg = "constructor: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_enable_shared_cache(1);
  if(rc != SQLITE_OK)
    logger.warn("enable_shared_cache failed");
  
  logger.setIgnoreLevel(Level::TRACE);
}

sqlite::~sqlite()
{
  sqlite3_close(db);
}

void sqlite::add_links(std::vector<std::string> links)
{
  std::string protocol;
  std::string domain;
  std::string path;
  std::size_t found;
  
  std::string sql;
  int rc;
  char *err = 0;
  
  sql = "BEGIN";
  rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  if(rc != SQLITE_OK)
    logger.error("BEGIN failed");
  
  for(auto &link : links)
  {
    if( (found = link.find("://")) != std::string::npos)
    {
      protocol = link.substr(0, found);
      link = link.substr(found + 3, link.length());
    } else {
      protocol = "http"; // assume http
    }
    
    if(protocol != "http" && protocol != "https")
    {
      logger.trace("SQLite: Dropping: " + link);
      continue;
    }
    
    if( (found = link.find("/")) != std::string::npos)
    {
      domain = link.substr(0, found);
      path = link.substr(found, link.length());
    } else {
      domain = link;
      path = "/";
    }
    
    path = boost::algorithm::replace_all_copy(path, "'", "''");
    
    sql = "INSERT INTO Links (domain,path,protocol) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "');";
      
    logger.trace("SQL: " + sql);

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    
    if(rc != SQLITE_OK)
    {
      if(rc != SQLITE_CONSTRAINT) // Alread in DB
      {
        std::string errmsg = "add_links: ";
        errmsg.append(sqlite3_errstr(rc));
        errmsg.append(" " + sql);
        throw(CrawlerException(errmsg));
      }
    }
  }
  
  sql = "COMMIT";
  rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  if(rc != SQLITE_OK)
    logger.error("COMMIT failed");
  
  return;
}

bool sqlite::get_visited(
  std::string domain, 
  std::string path, 
  std::string protocol)
{
  sqlite3_stmt *statement;
  
  std::string sql = "SELECT visited FROM Links WHERE domain = '" \
    + domain + "' AND path = '" + path + "' AND protocol = '" + protocol \
    + "';";
    
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "get_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_ROW)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "get_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  bool ret = sqlite3_column_int(statement, 0);
  sqlite3_finalize(statement);
  return ret;
}

void sqlite::set_visited(
  std::string domain,
  std::string path,
  std::string protocol)
{
  sqlite3_stmt *statement;
  
  std::string sql = "UPDATE Links SET visited = '1' WHERE domain = '" \
    + domain + "' AND PATH = '" + path + "' AND protocol = '" + protocol + "';";
    
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "sql_set_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_DONE)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "sql_set_vist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  sqlite3_finalize(statement);
  set_last_visited(domain, path, protocol);
}

void sqlite::set_last_visited(
  std::string domain,
  std::string path,
  std::string protocol)
{
  using namespace std::chrono;
  system_clock::time_point tp = system_clock::now();
  system_clock::duration dtn = tp.time_since_epoch();
  unsigned int seconds = dtn.count() * system_clock::period::num / system_clock::period::den;
  
  sqlite3_stmt *statement;
  std::string sql = "UPDATE Links SET lastVisited = '" + std::to_string(seconds) \
    + "' WHERE domain = '" + domain + "' AND path = '" + path + "' AND protocol = '" \
    + protocol + "';";
    
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "set_lvisit: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc != SQLITE_DONE)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "set_lvist: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  sqlite3_finalize(statement);
  return;
}

v_links sqlite::get_links(std::size_t num)
{
  v_links links;
  sqlite3_stmt *statement;
  
  std::string sql = "SELECT domain,path,protocol FROM Links WHERE visited = '0'" \
    "AND blacklisted = '0' LIMIT " + std::to_string(num) + ";";
    
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "get_links: ";
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
      std::string proto = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
      
      std::tuple<std::string,std::string,std::string> link(domain,path,proto);
      links.push_back(link);
      
      rc = sqlite3_step(statement);
    } else {
      sqlite3_finalize(statement);
      std::string errmsg = "sql_fill: ";
      errmsg.append(sqlite3_errstr(rc));
      throw(CrawlerException(errmsg));
    }
  }
  
  sqlite3_finalize(statement);
  return links;
}

void sqlite::blacklist(std::string domain, std::string path, std::string protocol)
{
  std::size_t found;
  if( (found = path.find("*")) != std::string::npos )
    return;
  if( (found = path.find("?")) != std::string::npos )
    return;
    
  std::string sql = "INSERT OR REPLACE INTO Links (domain,path,protocol,blacklisted) " \
    "VALUES ('" + domain + "', '" + path + "', '" + protocol + "', '" \
    + "1');";
  
  char *err = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    
  logger.debug("Blacklisted: " + domain + path + " (" + protocol + ")\n");
  
  if(rc != SQLITE_OK)
   {
    std::string errmsg = "sql_blacklist: ";
    errmsg.append(sqlite3_errstr(rc));
    errmsg.append(" " + sql);
    throw(CrawlerException(errmsg));
   }
   return;
}

void sqlite::set_robot(std::string domain)
{
  using namespace std::chrono;
  system_clock::time_point tp = system_clock::now();
  system_clock::duration dtn = tp.time_since_epoch();
  unsigned int seconds = dtn.count() * system_clock::period::num / system_clock::period::den;
  
  std::string sql = "INSERT OR REPLACE INTO RobotRules (domain,lastUpdated) " \
    "VALUES ('" + domain + "', '" + std::to_string(seconds) + "');";
  
  char *err = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  
  logger.debug("Adding " + domain + " to RobotRules");
  
  if(rc != SQLITE_OK)
   {
    std::string errmsg = "sql_robot: ";
    errmsg.append(sqlite3_errstr(rc));
    errmsg.append(" " + sql);
    throw(CrawlerException(errmsg));
   }
   
  return;
}

bool sqlite::should_process_robots(std::string domain)
{
  bool ret;
  sqlite3_stmt *statement;
  std::string sql = "SELECT domain FROM RobotRules WHERE domain = '" + domain + "';";
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "sql_prcRobot: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  if(rc == SQLITE_ROW)
    ret = false;
  else if(rc == SQLITE_DONE)
    ret = true;
  else
  {
    sqlite3_finalize(statement);
    std::string errmsg = "sql_prcRobot: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  sqlite3_finalize(statement);
  return ret;
}
