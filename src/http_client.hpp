/*
 * WebCrawler: http_client.h
 * 
 * Copyright 2014 Kyle Givler <xvegan88x@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

using boost::asio::ip::tcp;
using namespace boost;

static const bool DEBUG = true;

class http_client
{
  public:
    http_client();
    virtual ~http_client();
  
  private:
   tcp::resolver resolver;
   tcp::socket socket;
};

#endif /* HTTP_CLIENT_H */ 
