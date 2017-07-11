//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_CONNECTION_MANAGER_HPP
#define HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include "connection.hpp"
#include <boost/thread.hpp>
namespace http {
namespace server {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
{
public:
  connection_manager(const connection_manager&) = delete;
  connection_manager& operator=(const connection_manager&) = delete;

  /// Construct a connection manager.
  connection_manager();

  /// Add the specified connection to the manager and start it.
  void start(connection_ptr c);

  /// Stop the specified connection.
  void stop(connection_ptr c);

  /// Stop all connections.
  void stop_all();

  bool getRequestConnects(std::set<connection_ptr>& vOut);
  void addRequestConnects( connection_ptr ptr );
private:
  /// The managed connections.
  std::set<connection_ptr> connections_;

  std::set<connection_ptr> reqConnections;
  boost::mutex mtx;
};

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_MANAGER_HPP
