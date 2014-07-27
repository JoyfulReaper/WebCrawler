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
  : databaseFile(databaseFile)
{
  int rc = sqlite3_open(databaseFile.c_str());
  if(rc)
    throw("Can't open database: " << databaseFile);
}

sqlite_database::~sqlite_database()
{
  if(db)
    sqlite3_close(db);
}
