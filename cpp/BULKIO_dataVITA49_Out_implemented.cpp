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

#include "BULKIO_dataVITA49_Out_implemented.h"

// ----------------------------------------------------------------------------------------
// BULKIO_dataVITA49_Out_i definition
// ----------------------------------------------------------------------------------------
BULKIO_dataVITA49_Out_implemented::BULKIO_dataVITA49_Out_implemented(std::string port_name, SinkVITA49_base *_parent) :
BULKIO_dataVITA49_Out_i(port_name, _parent)
{
    parent = static_cast<SinkVITA49_i *> (_parent);
    recConnectionsRefresh = false;
    recConnections.length(0);
}

BULKIO_dataVITA49_Out_implemented::~BULKIO_dataVITA49_Out_implemented()
{
}

/*
 * pushSRI
 *     description: send out SRI describing the data payload
 *
 *  H: structure of type BULKIO::StreamSRI with the SRI for this stream
 *    hversion
 *    xstart: start time of the stream
 *    xdelta: delta between two samples
 *    xunits: unit types from Platinum specification
 *    subsize: 0 if the data is one-dimensional
 *    ystart
 *    ydelta
 *    yunits: unit types from Platinum specification
 *    mode: 0-scalar, 1-complex
 *    streamID: stream identifier
 *    sequence<CF::DataType> keywords: unconstrained sequence of key-value pairs for additional description
 */
void BULKIO_dataVITA49_Out_implemented::pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T)
{
    std::vector < std::pair < BULKIO::dataVITA49_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->pushSRI(H, T);
            } catch(...) {
                std::cout << "Call to pushSRI by BULKIO_dataVITA49_Out_implemented failed" << std::endl;
            }
        }
    }

    currentSRIs[std::string(H.streamID)] = H;
    refreshSRI = false;

    return;
}


void BULKIO_dataVITA49_Out_implemented::attach(const BULKIO::VITA49StreamDefinition& stream, const char* userid){
	boost::mutex::scoped_lock lock(updatingPortsLock);
	std::string attachId;
	std::string user_id(userid);
	std::map<BULKIO::dataVITA49::_var_type, std::string>::iterator portIter;
	BULKIO::dataVITA49::_var_type port = NULL;
	//lastStreamData = new BULKIO::VITA49StreamDefinition(stream);
	portIter = attachedPorts.begin();
	while (portIter != attachedPorts.end()) {
		port = (*portIter).first;
		port->detach(attachedPorts[port].c_str());
		attachedGroup.erase((*portIter).second);
		portIter++;
	}
	std::vector< std::pair<BULKIO::dataVITA49::_var_type, std::string> >::iterator portIter2 = outConnections.begin();
	while (portIter2 != outConnections.end()) {
		port = (*portIter2).first;
		attachId = port->attach(stream, user_id.c_str());
		attachedGroup.insert(std::make_pair(attachId, std::make_pair(lastStreamData, user_id)));
		attachedPorts[port] = attachId;
		portIter2++;
	}
}

void BULKIO_dataVITA49_Out_implemented::detach(){
	boost::mutex::scoped_lock lock(updatingPortsLock);
	std::vector< std::pair<BULKIO::dataVITA49::_var_type, std::string> >::iterator portIter = outConnections.begin();
	std::map<BULKIO::dataVITA49::_var_type, std::string>::iterator portIter2;
	while (portIter != outConnections.end()) {
		portIter2 = attachedPorts.begin();
			while (portIter2 != attachedPorts.end()) {
				if ((*portIter2).first == (*portIter).first) {
					(*(*portIter).first).detach(attachedPorts[(*portIter).first].c_str());
					return;
				}
				portIter2++;
		}
		portIter++;
	}
	attachedPorts.clear();
}

