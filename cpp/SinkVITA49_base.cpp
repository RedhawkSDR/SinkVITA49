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

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

#include "SinkVITA49_base.h"

SinkVITA49_base::SinkVITA49_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    dataShort_in = new bulkio::InShortPort("dataShort_in");
    addPort("dataShort_in", dataShort_in);
    dataFloat_in = new bulkio::InFloatPort("dataFloat_in");
    addPort("dataFloat_in", dataFloat_in);
    dataChar_in = new bulkio::InCharPort("dataChar_in");
    addPort("dataChar_in", dataChar_in);
    dataDouble_in = new bulkio::InDoublePort("dataDouble_in");
    addPort("dataDouble_in", dataDouble_in);
    dataOctet_in = new bulkio::InOctetPort("dataOctet_in");
    addPort("dataOctet_in", dataOctet_in);
    dataUshort_in = new bulkio::InUShortPort("dataUshort_in");
    addPort("dataUshort_in", dataUshort_in);
    dataVITA49_out = new bulkio::OutVITA49Port("dataVITA49_out");
    addPort("dataVITA49_out", dataVITA49_out);
}

SinkVITA49_base::~SinkVITA49_base()
{
    delete dataShort_in;
    dataShort_in = 0;
    delete dataFloat_in;
    dataFloat_in = 0;
    delete dataChar_in;
    dataChar_in = 0;
    delete dataDouble_in;
    dataDouble_in = 0;
    delete dataOctet_in;
    dataOctet_in = 0;
    delete dataUshort_in;
    dataUshort_in = 0;
    delete dataVITA49_out;
    dataVITA49_out = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void SinkVITA49_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void SinkVITA49_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void SinkVITA49_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void SinkVITA49_base::loadProperties()
{
    addProperty(network_settings,
                network_settings_struct(),
                "network_settings",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(VITA49Encapsulation,
                VITA49Encapsulation_struct(),
                "VITA49Encapsulation",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(VITA49IFDataPacket,
                VITA49IFDataPacket_struct(),
                "VITA49IFDataPacket",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(VITA49IFContextPacket,
                VITA49IFContextPacket_struct(),
                "VITA49IFContextPacket",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(advanced_configuration,
                advanced_configuration_struct(),
                "advanced_configuration",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(connection_status,
                connection_status_struct(),
                "connection_status",
                "",
                "readonly",
                "",
                "external",
                "configure");

}


