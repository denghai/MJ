//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_manager.hpp"

namespace http {
namespace server {

connection_manager::connection_manager()
{
}

void connection_manager::start(connection_ptr c)
{
  connections_.insert(c);
  c->start();
}

void connection_manager::stop(connection_ptr c)
{
  connections_.erase(c);
  c->stop();
}

void connection_manager::stop_all()
{
  for (auto c: connections_)
    c->stop();
  connections_.clear();
}

bool connection_manager::getRequestConnects(std::set<connection_ptr>& vOut)
{
	vOut.clear();
	boost::lock_guard<boost::mutex> lock(this->mtx);
	reqConnections.swap(vOut);
	return vOut.empty() == false;
}

void connection_manager::addRequestConnects(connection_ptr ptr)
{
	boost::lock_guard<boost::mutex> lock(this->mtx);
	reqConnections.insert(ptr);
}

} // namespace server
} // namespace http
