/*
    Logger: logger.cxx
    Copyright (C) 2014 Kyle Givler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

/**
* @file logger.cxx
* @author Kyle Givler
* 
* Provide logging mechanism
* Version 0.1
*/

#include "logger.hpp"

Logger::Logger(std::string name, Level level, std::ostream &initStream) :
  name(name),
  logLevel(level)
{
  addStream(initStream);
}
  
void Logger::log(std::string message)
{
  log(getLevel(), message);
}

void Logger::log(Level level, std::string message)
{
  if (level <= ignoreLevel || !enabled) // Not interested in this priority message
    return;
  
  std::string sLevel;
  
  switch (level)
  {
    case Level::TRACE:
      sLevel = "TRACE";
      break;
    case Level::DEBUG:
      sLevel = "DEBUG";
      break;
    case Level::INFO:
      sLevel = "INFO";
      break;
    case Level::WARN:
      sLevel = "WARN";
      break;
    case Level::ERROR:
      sLevel = "ERROR";
      break;
    case Level::SEVERE:
      sLevel = "SEVERE";
      break;
    default:
      sLevel = "INVALID";
  }
  
  auto it = streams.begin();
  while(it != streams.end())
  {
    **it << "LOGGER(" << name << ":" << sLevel << "): " << message << std::endl;
    ++it;
  }
}

void Logger::setLevel(Level level)
{
  this->logLevel = level;
}

Level Logger::getLevel() const
{
  return logLevel;
}

void Logger::setIgnoreLevel(Level level)
{
  ignoreLevel = level;
}

Level Logger::getIgnoreLevel()
{
  return ignoreLevel;
}

std::string Logger::getName() const
{
  return name;
}
  
bool Logger::addStream(std::ostream &o)
{
  streams.push_back(&o);
  return true;
}

bool Logger::removeStream(std::ostream &o)
{
  auto it = streams.begin();
  while (it != streams.end())
  {
    if (*it == &o)
    {
      streams.erase(it);
      return true;
    }
    ++it;
  }
  return false;
}

void Logger::enable()
{
  enabled = true;
}

void Logger::disable()
{
  enabled = false;
}

bool Logger::isEnabled()
{
  return enabled;
}
