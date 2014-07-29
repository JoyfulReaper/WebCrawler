/*
    Logger: logger.hpp
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
 * @file Logger.hpp
 * @author Kyle Givler
 * 
 * Provide logging mechanism
 * Version 0.1
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <vector>
#include <iostream>

enum class Level {NONE, TRACE, DEBUG, INFO, WARN, ERROR, SEVERE};

class Logger
{
public:
  /**
   * Constructor:
   * @param name The name of the Logger
   * @param level The default logging level
   * @param initStream The initial stream to log to
   */
  Logger(std::string name = "Logger", Level level = Level::WARN, std::ostream &initStream = std::cerr);
  Logger(const Logger& copy) = delete; // Disable copy Constructor
  
  
  /**
   * Log message to output streams
   * @param message The message to log at default level
   */
  void log(std::string message);
  
  /**
   * Log message to output streams
   * @param level the priority level of the message
   * @param message The message to Log
   */
  void log(Level level, std::string message);
  
  void trace(std::string msg)
  {
    log(Level::TRACE, msg);
  }
  
  void debug(std::string msg)
  {
    log(Level::DEBUG, msg);
  }
  
  void info(std::string msg)
  {
    log(Level::INFO, msg);
  }
  
  void warn(std::string msg)
  {
    log(Level::WARN, msg);
  }
  
  void error(std::string msg)
  {
    log(Level::ERROR, msg);
  }
  
  void severe(std::string msg)
  {
    log(Level::SEVERE, msg);
  }
  
  /**
   * @param level Default logging level
   */
  void setLevel(Level level);
  
  /**
   * @return The current default logging level
   */
  Level getLevel() const;
  
  /**
   * @param level messages with <= levels are ignored
   */
  void setIgnoreLevel(Level level);
  
  /**
   * @return current ignore level
   */
  Level getIgnoreLevel();
  
  /**
   * @return the name of this Logger
   */
  std::string getName() const;
  
  /**
   * Add a new output stream
   * The log message is sent to all added streams
   * @return true on sucess, false on failure
   */
  bool addStream(std::ostream &o);
  
  /**
   * @param o the stream to be removed
   */
  bool removeStream(std::ostream &o);
  
  /**
   * Enable Logging
   */
  void enable();
  
  /**
   * Disable Logging
   */
  void disable();
  
  /**
   * Check if logging is enabled
   * @return true if enabled, false if disabled
   */
  bool isEnabled();
  
private:
  std::vector<std::ostream*> streams;
  std::string name; // Name of the Logger
  Level logLevel; // Current logging level
  Level ignoreLevel = Level::DEBUG; // Ignore this level and lower
  bool enabled = true; // Is logging enabled?
};

#endif
