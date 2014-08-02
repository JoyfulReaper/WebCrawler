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
#include <boost/algorithm/string/case_conv.hpp>

sqlite::sqlite(std::string databaseFile)
  : databaseFile(databaseFile),
    logger("sqlite")
{
  
  // Check if table exists:
  // SELECT name FROM sqlite_master WHERE type='table' AND name = 'Links'
  
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
  logger.debug("~Closing database");
  int rc = sqlite3_close_v2(db);
  if(rc != SQLITE_OK)
    logger.error("Unable to close DB");
}

void sqlite::close_db()
{
  logger.debug("Closing database");
  int rc = sqlite3_close_v2(db);
  if(rc != SQLITE_OK)
    logger.error("Unable to close DB");
}

void sqlite::add_links(std::vector<std::string> links)
{
  logger.debug("Adding links to DB");
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
      logger.debug("SQLite: Dropping: (" + protocol + ")" + link);
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
    boost::to_lower(domain);
    boost::to_lower(protocol);
    
    sql = "INSERT INTO Links (domain,path,protocol) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "');";
      
    logger.trace("Adding link to DB: " + protocol + "://" + domain + path);

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    
    if(rc != SQLITE_OK)
    {
      if(rc != SQLITE_CONSTRAINT) // Alread in DB
      {
        std::string errmsg = "add_links: ";
        errmsg.append(sqlite3_errstr(rc));
        errmsg.append(" " + sql);
        throw(CrawlerException(errmsg));
      } else {
        logger.trace("Already in DB: " + domain + path);
      }
    }
  }
  
  sql = "COMMIT";
  rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  if(rc != SQLITE_OK)
    logger.error("COMMIT failed");
    
  sqlite3_free(err);
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
  std::string protocol,
  unsigned int code)
{
  sqlite3_stmt *statement;
  
  path = boost::algorithm::replace_all_copy(path, "'", "''");
  
  std::string sql = "UPDATE Links SET visited = '1', lastCode = '" + \
    std::to_string(code) + "' WHERE domain = '" + domain + "' AND PATH = '" \
    + path + "' AND protocol = '" + protocol + "';";
  
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
    " LIMIT " + std::to_string(num) + ";";
    
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "get_links: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg + " " + sql));
  }
  
  rc = sqlite3_step(statement);
  while(rc != SQLITE_DONE)
  {
    if(rc == SQLITE_ROW)
    {
      std::string domain = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
      std::string path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
      std::string proto = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
      
      if(check_blacklist(domain, path, proto))
      {
        remove_link(domain, path, proto);
        rc = sqlite3_step(statement);
        continue;
      }
      
      std::tuple<std::string,std::string,std::string> link(domain,path,proto);
      links.push_back(link);
      
      rc = sqlite3_step(statement);
    } else {
      sqlite3_finalize(statement);
      std::string errmsg = "get_links: ";
      errmsg.append(sqlite3_errstr(rc));
      throw(CrawlerException(errmsg));
    }
  }
  
  sqlite3_finalize(statement);
  return links;
}

bool sqlite::check_blacklist(
  std::string domain, 
  std::string path, 
  std::string proto)
{
  bool blacklisted = false;
  sqlite3_stmt *statement;
  std::string sql = "SELECT domain,path,protocol FROM Blacklist WHERE " \
    "domain = '" + domain + "' AND protocol = '" + proto + "';";
  
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &statement, 0);
  if(rc != SQLITE_OK)
  {
    sqlite3_finalize(statement);
    std::string errmsg = "check_bl: ";
    errmsg.append(sqlite3_errstr(rc));
    throw(CrawlerException(errmsg));
  }
  
  rc = sqlite3_step(statement);
  while(rc != SQLITE_DONE)
  {
    if(rc == SQLITE_ROW)
    {
      std::string bl_domain = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
      std::string bl_path = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
      std::string bl_proto = reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
      
      if(bl_domain == domain && bl_path == path && bl_proto == proto)
      {
        logger.info("Hit exact match blacklist");
        blacklisted = true; // Exact match
      }
      
      //std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ "<< bl_path << "\n";
      if(bl_domain == domain && bl_path == "/" && bl_proto == proto)
      {
        logger.info("Hit whole site blackliste");
        blacklisted = true; // Whole site
      }
        
      if(bl_domain == domain && bl_proto == proto)
      {
        std::size_t found;
        if( ( found = path.find(bl_path) ) != std::string::npos)
        {
          if( (found == 0) && (path.back() == '/') )
          {
            blacklisted = true; // Directory
            logger.info("Hit directory blacklist: " + proto + "://" + domain + path);
          }
        }
      }
      
      rc = sqlite3_step(statement);
    } else {
      sqlite3_finalize(statement);
      std::string errmsg = "check_bl: ";
      errmsg.append(sqlite3_errstr(rc));
      throw(CrawlerException(errmsg));
    }
  }
  
  sqlite3_finalize(statement);
  
  if(blacklisted)
    logger.debug("Hit blacklist: " + proto +"://" + domain + path);
  
  return blacklisted;
}

void sqlite::remove_link(std::string domain, std::string path, std::string protocol)
{
  std::string sql = "DELETE FROM Links WHERE domain = '" + domain + "' AND path = '" \
    + path + "' AND protocol = '" + protocol +"';";
    
  logger.warn("REMOVING LINK: " + protocol + "://" + domain + path + " !!!!!!!!!!!!!!!!!!!!!!!!!!!");
  
  char *err = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  
  if(rc != SQLITE_OK)
  {
    std::string errmsg = "remove_link: ";
    errmsg.append(sqlite3_errstr(rc));
    errmsg.append(" " + sql);
    throw(CrawlerException(errmsg));
  }
}

void sqlite::blacklist(
  std::string domain, 
  std::string path, 
  std::string protocol,
  std::string reason)
{
  path = boost::algorithm::replace_all_copy(path, "'", "''");
  
  std::string sql = "INSERT OR REPLACE INTO Blacklist (domain,path,protocol,reason) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "', '" \
      + reason + "');";

  logger.info("Blacklisting: " + protocol + "://" + domain + path + " ( " + reason + ")");

  char *err = 0;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
  if(rc != SQLITE_OK)
  {
    std::string errmsg = "sql_blacklist_single: ";
    errmsg.append(sqlite3_errstr(rc));
    errmsg.append(" " + sql);
    throw(CrawlerException(errmsg));
  }
  sqlite3_free(err);
}

void sqlite::blacklist(v_links blacklist, std::string reason)
{
  std::string sql = "BEGIN";
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
  if(rc != SQLITE_OK)
    logger.error("BEGIN failed");
    
  for(auto &link : blacklist)
  {
    std::string domain = std::get<0>(link);
    std::string path = std::get<1>(link);
    std::string protocol = std::get<2>(link);
    
    std::size_t found;
    if( (found = path.find("*")) != std::string::npos )
      continue;
    if( (found = path.find("?")) != std::string::npos )
      continue; // We don't add these anyway
    
    path = boost::algorithm::replace_all_copy(path, "'", "''");
    
    sql = "INSERT OR REPLACE INTO Blacklist (domain,path,protocol,reason) " \
      "VALUES ('" + domain + "', '" + path + "', '" + protocol + "', '" \
      + reason + "');";
  
    char *err = 0;
    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);
    
    logger.info("Blacklisted: " + domain + path + " (" + protocol + ")");
  
    if(rc != SQLITE_OK)
    {
      std::string errmsg = "sql_blacklist: ";
      errmsg.append(sqlite3_errstr(rc));
      errmsg.append(" " + sql);
      throw(CrawlerException(errmsg));
    }
  }
  sql = "COMMIT";
  rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
  if(rc != SQLITE_OK)
    logger.error("COMMIT failed");
    
   return;
}

void sqlite::set_robot_processed(std::string domain, std::string protocol)
{
  using namespace std::chrono;
  system_clock::time_point tp = system_clock::now();
  system_clock::duration dtn = tp.time_since_epoch();
  unsigned int seconds = dtn.count() * system_clock::period::num / system_clock::period::den;
  
  std::string sql = "INSERT OR REPLACE INTO RobotRules (domain,protocol,lastUpdated) " \
    "VALUES ('" + domain + "', '" + protocol + "', '" + std::to_string(seconds) + "');";
  
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
  sqlite3_free(err);
  return;
}

bool sqlite::should_process_robots(std::string domain, std::string protocol)
{
  bool ret;
  sqlite3_stmt *statement;
  std::string sql = "SELECT domain FROM RobotRules WHERE domain = '" + domain + "'" \
    "AND protocol = '" + protocol + "';";
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
