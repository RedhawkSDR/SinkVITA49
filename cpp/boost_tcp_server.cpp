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

#include <omniORB4/CORBA.h>
#include "boost_tcp_server.h"

void session::start()
{
	socket_.async_read_some(boost::asio::buffer(read_data_, max_length_),
			boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

template<typename T, typename U>
void session::write(std::vector<T, U>& data)
{
	if (socket_.is_open())
	{
		boost::mutex::scoped_lock lock(writeLock_);
		size_t numBytes = data.size()*sizeof(T);
		writeBuffer_.push_back(std::vector<char>(numBytes));
		memcpy(&writeBuffer_.back()[0],&data[0],numBytes);
		if (writeBuffer_.size()==1)
		{
			boost::asio::async_write(socket_,
				boost::asio::buffer(writeBuffer_[0]),
				boost::bind(&session::handle_write, shared_from_this(),
						boost::asio::placeholders::error));
		}
	}
}

void session::handle_read(const boost::system::error_code& error,
		size_t bytes_transferred)
{
	if (!error)
	{
		read_data_.resize(bytes_transferred);
        this->_printDebug("TCPServer::session::handle_read - Giving session data to server");
		server_->newSessionData(read_data_);
		read_data_.resize(max_length_);
		socket_.async_read_some(boost::asio::buffer(read_data_, max_length_),
				boost::bind(&session::handle_read, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	else
	{
        std::stringstream ss;
        ss << "TCPServer::session::handle_read - Error occurred: " << error;
        this->_printDebug(ss.str());
	}
}

void session::handle_write(const boost::system::error_code& error)
{
	boost::mutex::scoped_lock lock(writeLock_);
	writeBuffer_.pop_front();
	if (error)
	{
		std::cerr<<"TCPServer::session - ERROR writing session data: "<<error<<std::endl;
		server_->closeSession(shared_from_this());
	}
	else if(!writeBuffer_.empty())
	{
        this->_printDebug("TCPServer::session::handle_write - Continuing write...");
		boost::asio::async_write(socket_,
						boost::asio::buffer(writeBuffer_[0]),
						boost::bind(&session::handle_write, shared_from_this(),
								boost::asio::placeholders::error));
	}
}

template<typename T, typename U>
void server::write(std::vector<T, U>& data)
{
	boost::mutex::scoped_lock lock(sessionsLock_);
	for (std::list<session_ptr>::iterator i = sessions_.begin(); i!=sessions_.end(); i++)
	{
        this->_printDebug("TCPServer::write - Writing to session");
		session_ptr thisSession= *i;
		thisSession->write(data);
	}
}

template<typename T>
void server::read(std::vector<char, T> & data, size_t index)
{
	boost::mutex::scoped_lock lock(pendingDataLock_);
	int numRead=std::min(data.size()-index, pendingData_.size());
	data.resize(index+numRead);
	int j=0;
	for (unsigned int i=index; i!=data.size(); i++)
	{
		data[i]=pendingData_[j];
		j++;
	}
	pendingData_.erase(pendingData_.begin(), pendingData_.begin()+numRead);
}

bool server::is_connected()
{
	return !sessions_.empty();
}

template<typename T>
void server::newSessionData(std::vector<char, T>& data)
{
	boost::mutex::scoped_lock lock(pendingDataLock_);
	int oldSize=pendingData_.size();
	pendingData_.resize(oldSize+data.size());
	char* newData= &data[0];
	for (unsigned int i=oldSize; i!=pendingData_.size(); i++)
	{
		pendingData_[i]=*newData;
		newData++;
	}
}
void server::closeSession(session_ptr ptr)
{
	boost::mutex::scoped_lock lock(sessionsLock_);
	for (std::list<session_ptr>::iterator i=sessions_.begin(); i!=sessions_.end(); i++)
	{
		if (ptr==*i)
		{
			sessions_.remove(ptr);
			break;
		}
	}
}


void server::start_accept()
{
    this->_printDebug("TCPServer::start_accept - Start accepting connections");
	{
        // Need to save session as local member... Going out of scope leads to
        // issues
        session_ptr new_session(new session(io_service_, this, maxLength_, debug));
        sessionWaitingForAccept_ = new_session;
		acceptor_.async_accept(sessionWaitingForAccept_->socket(),
				boost::bind(&server::handle_accept, this, boost::asio::placeholders::error));
	}
}

void server::handle_accept(const boost::system::error_code& error)
{
		if (!error)
		{
			{
				boost::mutex::scoped_lock lock(sessionsLock_);
				sessions_.push_back(sessionWaitingForAccept_);
			}
            this->_printDebug("TCPServer::handle_accept - Starting new session");
			sessionWaitingForAccept_->start();
		}
		start_accept();
}


void server::run()
{
	try
	{
		io_service_.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "TCPServer:: Exception in thread: " << e.what() << std::endl;
	}
}
