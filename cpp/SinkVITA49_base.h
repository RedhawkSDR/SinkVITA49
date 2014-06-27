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

#ifndef SINKVITA49_IMPL_BASE_H
#define SINKVITA49_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>
#include "struct_props.h"

class SinkVITA49_base : public Resource_impl, protected ThreadedComponent
{
    public:
        SinkVITA49_base(const char *uuid, const char *label);
        ~SinkVITA49_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        network_settings_struct network_settings;
        VITA49Encapsulation_struct VITA49Encapsulation;
        VITA49IFDataPacket_struct VITA49IFDataPacket;
        VITA49IFContextPacket_struct VITA49IFContextPacket;
        advanced_configuration_struct advanced_configuration;
        connection_status_struct connection_status;

        // Ports
        bulkio::InShortPort *dataShort_in;
        bulkio::InFloatPort *dataFloat_in;
        bulkio::InCharPort *dataChar_in;
        bulkio::InDoublePort *dataDouble_in;
        bulkio::InOctetPort *dataOctet_in;
        bulkio::InUShortPort *dataUshort_in;
        bulkio::OutVITA49Port *dataVITA49_out;

    private:
};
#endif // SINKVITA49_IMPL_BASE_H
