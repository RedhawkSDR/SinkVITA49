/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components sourcesocket.
 *
 * REDHAWK Basic Components sourcesocket is free software: you can redistribute it and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components sourcesocket is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef BOOST_TCP_SERVER_H_
#define BOOST_TCP_SERVER_H_

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <deque>
#include "debuggable.h"

using boost::asio::ip::tcp;

class server;

class session : public Debuggable, public boost::enable_shared_from_this<session>
{
public:
	session(boost::asio::io_service& io_service, server* s, size_t max_length, bool debug=false)
	: Debuggable(debug),
      socket_(io_service),
	  server_(s),
	  read_data_(max_length),
	  max_length_(max_length)
	{
	}

    virtual ~session() {
        try {
            boost::system::error_code ec;
            this->socket_.shutdown(tcp::socket::shutdown_both, ec);
            if (ec) this->_printError("TCPServer::session::close - Failed to shutdown socket", ec);

            this->socket_.close(ec);
            if (ec) this->_printError("TCPServer::session::close - Failed to close socket", ec);
        } catch (std::exception& ex) {
            std::cerr << "TCPServer::session::~session - Exception occurred while closing socket: " << ex.what() << std::endl;
        }
    }

    void _printError(const std::string& msg, boost::system::error_code& ec) {
        std::stringstream ss;
        ss << msg << " - Error code: " << ec;
        this->_printDebug(ss.str());
    }

	tcp::socket& socket()
	{
		return socket_;
	}

	void start();

	template<typename T, typename U>
	void write(std::vector<T, U>& data);

    template<typename T>
    void write(T* data, size_t length)
    {
        if (socket_.is_open())
        {
            boost::mutex::scoped_lock lock(writeLock_);
            writeBuffer_.push_back(std::vector<char>(length));
            memcpy(&writeBuffer_.back()[0],data,length);
            if (writeBuffer_.size()==1)
            {
                boost::asio::async_write(socket_,
                    boost::asio::buffer(writeBuffer_[0]),
                    boost::bind(&session::handle_write, shared_from_this(),
                            boost::asio::placeholders::error));
            }
        }
    }




private:
	void handle_read(const boost::system::error_code& error,
			size_t bytes_transferred);

	void handle_write(const boost::system::error_code& error);

	tcp::socket socket_;
	server* server_;
	std::vector<char> read_data_;
	size_t max_length_;
	std::deque<std::vector<char> > writeBuffer_;
	boost::mutex writeLock_;
};


typedef boost::shared_ptr<session> session_ptr;


class server : public Debuggable
{
public:
	server(short port, size_t maxLength=1024, bool debug=false) :
        Debuggable(debug),
        thread_(NULL),
		acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)),
		maxLength_(maxLength),
        debug(debug)
	{
		start_accept();
		thread_ = new boost::thread(boost::bind(&server::run, this));
        this->_printDebug("TCPServer::constructor - Started thread for io_service");
	}

	~server()
	{
        {
          boost::mutex::scoped_lock lock(sessionsLock_);
          sessions_.clear();
          this->_printDebug("TCPServer::destructor - Cleared sessions");
        }

        if(thread_) {
			io_service_.stop();
            this->_printDebug("TCPServer::destructor - Stopped io_service");
			thread_->join();
            this->_printDebug("TCPServer::destructor - Stopped server thread");
			delete thread_;
            thread_ = 0;
		}
	}

	template<typename T, typename U>
	void write(std::vector<T, U>& data);


    template<typename T>
    void write(T* data, size_t length)
    {
        boost::mutex::scoped_lock lock(sessionsLock_);
        for (std::list<session_ptr>::iterator i = sessions_.begin(); i!=sessions_.end(); i++)
        {
            std::stringstream ss;
            ss << "TCPServer::write - Writing to session - length: " << length;
            this->_printDebug(ss.str());
            session_ptr thisSession= *i;
            thisSession->write(data, length);
        }
    }

	template<typename T>
	void read(std::vector<char, T> & data, size_t index=0);
	bool is_connected();

	template<typename T>
	void newSessionData(std::vector<char, T>& data);
	void closeSession(session_ptr ptr);


private:
	void start_accept();
	void handle_accept(const boost::system::error_code& error);

    void run();

    boost::thread* thread_;
	boost::asio::io_service io_service_;
	tcp::acceptor acceptor_;
	std::list<session_ptr> sessions_;
	std::vector<char> pendingData_;
	boost::mutex sessionsLock_;
	boost::mutex pendingDataLock_;
    session_ptr sessionWaitingForAccept_;
	size_t maxLength_;
    bool debug;
};

#endif /* BOOST_TCP_SERVER_H_ */
