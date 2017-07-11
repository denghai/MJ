//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <asio.hpp>
#include "server.hpp"
#include "reply.hpp"
int main(int argc, char* argv[])
{

	for (uint32_t nIdx = 0; nIdx < 8; ++nIdx)
	{
		char acc, cName;
		acc = rand() % 50;
		if (acc <= 25)
		{
			acc = 'a' + acc;
		}
		else
		{
			acc = 'A' + (acc - 25);
		}
		std::cout << acc << std::endl;
	}


  try
  {
    // Check command line arguments.
    if (argc != 4)
    {
      std::cerr << "Usage: http_server <address> <port> <doc_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 .\n";
      //return 1;
    }

	std::set<int32_t> v;
	v.insert(3);
	v.erase(3);
	auto iter = v.find(3);
	if (iter == v.cend())
	{
		std::cout << "end \n";
	}
	v.erase(3);

    // Initialise the server. 
    http::server::server s("192.168.0.108", "5006", "D://abc/");
	 
    // Run the server until stopped.
    s.run();

	while (true)
	{
		std::set < http::server::connection_ptr >  tt;
		if (s.getRequestConnects(tt))
		{
			std::cout << "get a request \n";
			Sleep(5000);
			for (auto& ref : tt)
			{
				std::cout << "method : " << ref->getReqPtr()->method << "   data = " <<  ref->getReqPtr()->reqContent;
				*ref->getReplyPtr() = http::server::reply::stock_reply(http::server::reply::not_found);
				ref->doReply();
				std::cout << "\n do reply \n";
				Sleep(2000);
			}
		}
		Sleep(1000);
	}

  }
  catch (std::exception& e)
  { 
    std::cerr << "exception: " << e.what() << "\n";
  }
  return 0;
}
