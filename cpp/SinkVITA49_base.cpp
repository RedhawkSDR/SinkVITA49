/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.

 * This is the component code. This file contains the child class where
 * custom functionality can be added to the component. Custom
 * functionality to the base class can be extended here. Access to
 * the ports can also be done from this class
 */

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

 	Source: SinkVITA49_base.cpp
 	Generated on: Fri Jan 10 11:20:07 EST 2014
 	REDHAWK IDE
 	Version: 1.8.6
 	Build id: N201312130030

*******************************************************************************************/

#include "SinkVITA49_base.h"

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/
 
SinkVITA49_base::SinkVITA49_base(const char *uuid, const char *label) :
                                     Resource_impl(uuid, label), serviceThread(0) {
    construct();
}

void SinkVITA49_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
    dataShort_in = new BULKIO_dataShort_In_i("dataShort_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataShort_in);
    dataFloat_in = new BULKIO_dataFloat_In_i("dataFloat_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataFloat_in);
    dataChar_in = new BULKIO_dataChar_In_i("dataChar_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataChar_in);
    dataDouble_in = new BULKIO_dataDouble_In_i("dataDouble_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataDouble_in);
    dataOctet_in = new BULKIO_dataOctet_In_i("dataOctet_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataOctet_in);
    dataUshort_in = new BULKIO_dataUshort_In_i("dataUshort_in", this);
    oid = ossie::corba::RootPOA()->activate_object(dataUshort_in);
    dataVITA49_out = new BULKIO_dataVITA49_Out_i("dataVITA49_out", this);
    oid = ossie::corba::RootPOA()->activate_object(dataVITA49_out);

    registerInPort(dataShort_in);
    registerInPort(dataFloat_in);
    registerInPort(dataChar_in);
    registerInPort(dataDouble_in);
    registerInPort(dataOctet_in);
    registerInPort(dataUshort_in);
    registerOutPort(dataVITA49_out, dataVITA49_out->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void SinkVITA49_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void SinkVITA49_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        dataShort_in->unblock();
        dataFloat_in->unblock();
        dataChar_in->unblock();
        dataDouble_in->unblock();
        dataOctet_in->unblock();
        dataUshort_in->unblock();
        serviceThread = new ProcessThread<SinkVITA49_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void SinkVITA49_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        dataShort_in->block();
        dataFloat_in->block();
        dataChar_in->block();
        dataDouble_in->block();
        dataOctet_in->block();
        dataUshort_in->block();
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

CORBA::Object_ptr SinkVITA49_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {

        if (!strcmp(_id,"dataShort_in")) {
            BULKIO_dataShort_In_i *ptr = dynamic_cast<BULKIO_dataShort_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataShort::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataFloat_in")) {
            BULKIO_dataFloat_In_i *ptr = dynamic_cast<BULKIO_dataFloat_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataFloat::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataChar_in")) {
            BULKIO_dataChar_In_i *ptr = dynamic_cast<BULKIO_dataChar_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataChar::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataDouble_in")) {
            BULKIO_dataDouble_In_i *ptr = dynamic_cast<BULKIO_dataDouble_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataDouble::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataOctet_in")) {
            BULKIO_dataOctet_In_i *ptr = dynamic_cast<BULKIO_dataOctet_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataOctet::_duplicate(ptr->_this());
            }
        }
        if (!strcmp(_id,"dataUshort_in")) {
            BULKIO_dataUshort_In_i *ptr = dynamic_cast<BULKIO_dataUshort_In_i *>(p_in->second);
            if (ptr) {
                return BULKIO::dataUshort::_duplicate(ptr->_this());
            }
        }
    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void SinkVITA49_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    delete(dataShort_in);
    delete(dataFloat_in);
    delete(dataChar_in);
    delete(dataDouble_in);
    delete(dataOctet_in);
    delete(dataUshort_in);
    delete(dataVITA49_out);
 
    Resource_impl::releaseObject();
}

void SinkVITA49_base::loadProperties()
{
    addProperty(network_settings,
               "network_settings",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(VITA49Encapsulation,
               "VITA49Encapsulation",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(VITA49IFDataPacket,
               "VITA49IFDataPacket",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(VITA49IFContextPacket,
               "VITA49IFContextPacket",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(advanced_configuration,
               "advanced_configuration",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(connection_status,
               "connection_status",
               "",
               "readonly",
               "",
               "external",
               "configure");

}
