/*
 * WebCrawler: request_reciver.hpp
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
 * @file http_request.hpp
 * @author Kyle Givler
 */

#ifndef _WC_REQUEST_RECIVER_H_
#define _WC_REQUEST_RECIVER_H_

#include "http_request.hpp"

class request_reciver
{
public:
  virtual ~request_reciver() {}
  
  virtual void receive_http_request(http_request *request) = 0;
private:
};

#endif
