/*
 * WebCrawler: crawlerException.hpp
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
 * @file crawlerException.hpp
 * @author Kyle Givler
 */

#ifndef _WC_CRAWLER_EXCEPTION_H_
#define _WC_CRAWLER_EXCEPTION_H_

#include <exception>
#include <iostream>
#include <string>

class CrawlerException : public std::exception
{
public:
  virtual ~CrawlerException() {}
  CrawlerException(std::string error) : msg(error)
  {
    msg.insert(0,"CrawlerException: ");
    msg.append("\n");
  }

  virtual const char *what() const throw()
  {
    return msg.c_str();
  }

private:
  std::string msg;
};

#endif
