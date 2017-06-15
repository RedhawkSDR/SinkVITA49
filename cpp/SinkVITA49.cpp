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

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 *******************************************************************************************/

#include "SinkVITA49.h"

PREPARE_LOGGING(SinkVITA49_i)

using namespace vrt;

// Protocol Field Sizes in bytes
// VITA49 IF Data and Context
const int VITA49_HEADER_SIZE = 4;           // Mandatory
const int VITA49_STREAM_ID_SIZE = 4;        // Optional
const int VITA49_CLASS_ID_SIZE = 8;         // Optional
const int VITA49_INT_SECS_SIZE = 4;         // Optional
const int VITA49_FRAC_SECS_SIZE = 8;        // Optional

// VITA49 IF Data Only
const int VITA49_TRAILER_SIZE = 4;          // Optional

// VITA49 Context Only
const int VITA49_CONTEXT_INDEX_SIZE = 4;    // Optional

// VITA49 VRL
const int VRL_FRAME_SIZE = 12;              // Optional for VRT, Mandatory for VRL  (?)
const int VRL_CRC_SIZE = 4;                 // Optional for VRT, Mandatory for VRL  (?)

// UDP
const int UDP_HEADER_SIZE = 8;              // Mandatory

// TCP
const int TCP_HEADER_SIZE = 20;             // Mandatory
// TCP options has range of 0-40 bytes      // Optional

/************************************************
 * Constructor
 *
 * Takes:  const char *uuid, const char *label
 ************************************************/
SinkVITA49_i::SinkVITA49_i(const char *uuid, const char *label) :
SinkVITA49_base(uuid, label) {
    __constructor__();
}

void SinkVITA49_i::__constructor__() {

    timeOut = 1;

    _transmitThread = NULL;
    _contextThread = NULL;
    //set ip address range for multicast
    lowMulti = inet_network("224.0.0.1");
    highMulti = inet_network("239.255.255.250");

    multicast = true;
    waitingForSRI = true;
    shouldUpdateStream = false;

    createMem = true;
    numBuffers = 200000;
    setDefaultSRI();
    waitForContext = true;
    tx_thead_running = false;
    pf = new PayloadFormat(true, RealComplexType_ComplexCartesian, DataItemFormat_Double, false, 0, 0, 64, 64, 1, 1);
    //initialize the _streamDef to some settings

    spareBuffer = NULL;

    // t(io, boost::posix_time::seconds(5));
    convertEndian = false;
    initialize_values();
    standardDPacket = new StandardDataPacket(100);
    
    resetCurrAttach();
    resetStreamMap();
    resetStreamDefinition();
    resetVITAProcess();

    unicast_udp_open = false;
    unicast_tcp_open = false;
    multicast_udp_open = false;
    
    this->dataVITA49_out->setLogger(this->__logger);
    
    // Setup property change listeners
    addPropertyChangeListener("network_settings", this, &SinkVITA49_i::networkSettingsChanged);
    addPropertyChangeListener("VITA49Encapsulation", this, &SinkVITA49_i::vita49EncapsulationChanged);
    addPropertyChangeListener("VITA49IFDataPacket", this, &SinkVITA49_i::vita49IFDataPacketChanged);
    addPropertyChangeListener("VITA49IFContextPacket", this, &SinkVITA49_i::vita49IFContextPacketChanged);
    addPropertyChangeListener("advanced_configuration", this, &SinkVITA49_i::advancedConfigurationChanged);
}

void SinkVITA49_i::resetCurrAttach() {
    curr_attach.eth_dev = "eth0";
    curr_attach.attach = false;
    curr_attach.ip_address = "127.0.0.1";
    curr_attach.manual_override = false;
    curr_attach.port = 0;
    curr_attach.vlan = 0;
    curr_attach.use_udp_protocol = true;
    curr_attach.attach_id = "";
}

void SinkVITA49_i::resetStreamMap() {
    _streamMap.hash = 0;
    _streamMap.streamID = "";
}

void SinkVITA49_i::resetStreamDefinition() {
    //_streamDef.ip_address = CORBA::string_dup("");
    _streamDef.ip_address = "127.0.0.1";
    _streamDef.vlan = 0;
    _streamDef.port = 0;
    _streamDef.protocol = BULKIO::VITA49_UDP_TRANSPORT;
    _streamDef.valid_data_format = false;
    _streamDef.data_format.packing_method_processing_efficient = true;
    _streamDef.data_format.complexity = BULKIO::VITA49_REAL;
    _streamDef.data_format.data_item_format = BULKIO::VITA49_32F;
    _streamDef.data_format.repeating = false;
    _streamDef.data_format.event_tag_size = 1;
    _streamDef.data_format.channel_tag_size = 1;
    _streamDef.data_format.item_packing_field_size = 32;
    _streamDef.data_format.data_item_size = 32;
    _streamDef.data_format.repeat_count = 1;
    _streamDef.data_format.vector_size = 1;
}

void SinkVITA49_i::resetVITAProcess() {
    //initalize the packet classes
    VITAProcess.IFCPacket.enable = true;
    VITAProcess.IFCPacket.enable_stream_identifier = true;
    VITAProcess.IFCPacket.stream_identifier_offset = 0;
   
    //need to reset this according the appendix A
    std::string class_identifier = "DEFAULT";
    std::string device_identifier = "FF-FF-FA:1301";
    VITAProcess.IFCPacket.enable_class_identifier = true;
    VITAProcess.IFCPacket.enable_device_identifier = true;

    VITAProcess.IFCPacket.class_identifier = CORBA::string_dup(class_identifier.c_str());
    VITAProcess.IFCPacket.device_identifier = CORBA::string_dup(device_identifier.c_str());
    
    VITAProcess.IFCPacket.embed_time_stamp = true;
    VITAProcess.IFCPacket.enable_trailer = false;

    VITAProcess.IFDPacket.enable = true;
    VITAProcess.IFDPacket.enable_stream_identifier = true;
    VITAProcess.IFDPacket.enable_class_identifier = true;
    
    VITAProcess.IFDPacket.embed_time_stamp = true;
    VITAProcess.IFDPacket.enable_trailer = false;
}

void SinkVITA49_i::initialize_values() {
    //    _streamMap.clear();

    burstPacketCount = 0;
    _attachMap.clear();
    remainingData = false;
    dataIndex = 0;
    packetCount = 0;
    contextCount = 0;
    samplesPerPacket = 0;
    standardPacketClassID = "";
    leftOverDataSize = 0;
    multicast = false;

    _tContext.tfsec = 0.0;
    _tContext.twsec = 0.0;
    _dataRef = BYTE_ORDER;
    _throttleTime = 0;
    vita49_payload_size = 1428;

    waitingForSRI = true;
}

/****************
 * Destructor
 ****************/
SinkVITA49_i::~SinkVITA49_i(void) {
    BasicVRTPacket* temp2;
    destroy_tx_thread();
    boost::mutex::scoped_lock lock1(BankLock);
    while (!Bank2.empty()) {
        temp2 = Bank2.front();
        Bank2.pop();
        delete temp2;
    }
    boost::mutex::scoped_lock lock2(workQueueLock);
    while (!workQueue2.empty()) {
        temp2 = workQueue2.front();
        workQueue2.pop();
        delete temp2;
    }
    if (spareBuffer != NULL)
        free(spareBuffer);
    delete pf;
   
    VITAProcess.IFCPacket.class_identifier = CORBA::string_dup("");
    VITAProcess.IFCPacket.device_identifier = CORBA::string_dup("");
}

void SinkVITA49_i::memoryManagement(int maxPacketLength) {
    destroy_tx_thread();
    if (spareBuffer != NULL)
        free(spareBuffer);
    spareBuffer = (char*) malloc(sizeof (char)*(maxPacketLength + 20 + 8));
    createMem = false;

}

void SinkVITA49_i::start() throw (CF::Resource::StartError, CORBA::SystemException) {
    boost::mutex::scoped_lock runLock(startstop_lock);
    SinkVITA49_base::start();
}

void SinkVITA49_i::stop() throw (CF::Resource::StopError, CORBA::SystemException) {
    boost::mutex::scoped_lock runLock(startstop_lock);

    SinkVITA49_base::stop();
    
    if (unicast_udp_open) {
        close(uni_server.sock);
        unicast_udp_open = false;
    }

    if (unicast_tcp_open) {
        unicast_tcp_open = false;
    }

    tearDownOutputStream();

}

void SinkVITA49_i::updateCurrAttach() {
    curr_attach.eth_dev = network_settings.interface;
    curr_attach.port = network_settings.port;
    curr_attach.ip_address = network_settings.ip_address;
    curr_attach.vlan = network_settings.vlan;
    curr_attach.use_udp_protocol = network_settings.use_udp_protocol;

    updateStreamDef();
}

void SinkVITA49_i::networkSettingsChanged(const network_settings_struct* oldVal,
                                          const network_settings_struct* newVal) {
    boost::mutex::scoped_lock lock(property_lock);
    if (started()) {
      if (network_settings.enable) {
        if (curr_attach.port != network_settings.port)
          LOG_INFO(SinkVITA49_i, "*** Received request to use port '"<<network_settings.port<<"' ***")
        if (curr_attach.eth_dev != network_settings.interface)
          LOG_INFO(SinkVITA49_i, "*** Received request to use interface '"<<network_settings.interface<<"' ***")
        if (curr_attach.ip_address != network_settings.ip_address)
          LOG_INFO(SinkVITA49_i, "*** Received request to use ip_address '"<<network_settings.ip_address<<"' ***")
        if (curr_attach.vlan != network_settings.vlan)
          LOG_INFO(SinkVITA49_i, "*** Received request to use vlan '"<<network_settings.vlan<<"' ***")
        if (curr_attach.use_udp_protocol != network_settings.use_udp_protocol) {
          if (network_settings.use_udp_protocol) {
            LOG_INFO(SinkVITA49_i, "*** Received request to enable udp protocol ***")
          } else {
            LOG_INFO(SinkVITA49_i, "*** Received request to disable udp protocol ***")
          }
      }
      }
    }
    shouldUpdateStream = true;
}

void SinkVITA49_i::vita49EncapsulationChanged(const VITA49Encapsulation_struct* oldVal,
                                              const VITA49Encapsulation_struct* newVal) {
    boost::mutex::scoped_lock lock(property_lock);
    VITAProcess.Encap.enable_crc = VITA49Encapsulation.enable_crc;
    VITAProcess.Encap.enable_vrl_frames = VITA49Encapsulation.enable_vrl_frames;   
}

void SinkVITA49_i::vita49IFDataPacketChanged(const VITA49IFDataPacket_struct* oldVal,
                                             const VITA49IFDataPacket_struct* newVal) {
    boost::mutex::scoped_lock lock(property_lock);
    if (VITA49IFDataPacket.enable) {
        VITAProcess.IFDPacket.enable = true;
        VITAProcess.IFDPacket.enable_class_identifier = VITA49IFDataPacket.enable_class_identifier;
        VITAProcess.IFDPacket.enable_stream_identifier = VITA49IFDataPacket.enable_stream_identifier;
        VITAProcess.IFDPacket.embed_time_stamp = VITA49IFDataPacket.embed_time_stamp;
        VITAProcess.IFDPacket.enable_trailer = VITA49IFDataPacket.enable_trailer;
    }
}

void SinkVITA49_i::vita49IFContextPacketChanged(const VITA49IFContextPacket_struct *oldVal,
                                                const VITA49IFContextPacket_struct *newVal) {
    boost::mutex::scoped_lock lock(property_lock);
    if (VITA49IFContextPacket.enable) {
        VITAProcess.IFCPacket.enable = true;
        VITAProcess.IFCPacket.enable_stream_identifier = VITA49IFContextPacket.enable_stream_identifier;
        VITAProcess.IFCPacket.stream_identifier_offset = VITA49IFContextPacket.stream_identifier_offset;
        VITAProcess.IFCPacket.enable_class_identifier = VITA49IFContextPacket.enable_class_identifier;  
        VITAProcess.IFCPacket.class_identifier = CORBA::string_dup(VITA49IFContextPacket.class_identifier.c_str());
        VITAProcess.IFCPacket.enable_device_identifier = VITA49IFContextPacket.enable_device_identifier;
        VITAProcess.IFCPacket.device_identifier = CORBA::string_dup(VITA49IFContextPacket.device_identifier.c_str());
        VITAProcess.IFCPacket.enable_stream_identifier = VITA49IFContextPacket.enable_stream_identifier; //
        VITAProcess.IFCPacket.embed_time_stamp = VITA49IFContextPacket.embed_time_stamp;
        VITAProcess.IFCPacket.enable_trailer = VITA49IFContextPacket.enable_trailer;
    }
}

void SinkVITA49_i::advancedConfigurationChanged(const advanced_configuration_struct *oldVal,
                                                const advanced_configuration_struct *newVal) {
    boost::mutex::scoped_lock lock(property_lock);
    int enumedType = advanced_configuration.endian_representation;

    if (enumedType == 1) {
        if (BYTE_ORDER != __LITTLE_ENDIAN) {
            _dataRef = __LITTLE_ENDIAN;
            convertEndian = true;
        }
    } else if (enumedType == 2) {
        if (BYTE_ORDER != __BIG_ENDIAN) {
            _dataRef = __BIG_ENDIAN;
            convertEndian = true;
        }
    }
    numBuffers = advanced_configuration.number_of_buffers;
    //UDP payload size cannot exceed 65507, this is the max UDP payload size
    if (advanced_configuration.max_payload_size >= 65503)
        vita49_payload_size = 65503;
    else
        vita49_payload_size = advanced_configuration.max_payload_size;

    _bulkioPriority = advanced_configuration.use_bulkio_sri;
    timeOut = advanced_configuration.time_between_context_packets;
    burstPacketCount = (int) advanced_configuration.number_of_packets_in_burst;
    _throttleTime = (int) advanced_configuration.throttle_time_between_packet_bursts;
    
    // Refresh TX thread when force transmit is enabled
    //if (advanced_configuration.force_transmit && !oldVal->force_transmit) {
    //    std::cout << "Destroying tx thread" << std::endl;
    //    destroy_tx_thread();
    //}
}

void SinkVITA49_i::destroy_tx_thread() {
    if (_transmitThread != NULL) {
        LOG_DEBUG(SinkVITA49_i, "DESTROYING TX THREAD");
        runThread = false;
        _transmitThread->join();
        delete _transmitThread;
        _transmitThread = NULL;
    }
    if (_contextThread != NULL) {
        LOG_DEBUG(SinkVITA49_i, "DESTROYING CONTEXT THREAD");
        runThread = false;
        _contextThread->join();
        delete _contextThread;
        _contextThread = NULL;
    }
    curr_attach.attach = false;
}

bool SinkVITA49_i::launch_tx_thread() {
    packetCount = 0;
    destroy_tx_thread();
    LOG_DEBUG(SinkVITA49_i, "STARTING TX THREAD");
    /* build the iterface string */
    std::ostringstream iface;
    iface << curr_attach.eth_dev;
    //connect to VLAN
    if (curr_attach.vlan != 0) {
        iface << "." << curr_attach.vlan;
    }

    in_addr_t attachedIP = inet_network(curr_attach.ip_address.c_str());
    const char *attachedIPstr = curr_attach.ip_address.c_str();
    std::string attachedInterfaceStr = iface.str();
    const char *attachedInterface = attachedInterfaceStr.c_str();

    //check to see if this a multicast address or not
    if (attachedIP > lowMulti && attachedIP < highMulti && !curr_attach.ip_address.empty()) {
        LOG_DEBUG(SinkVITA49_i, "Enabling multicast_client on " << attachedInterface << " " << attachedIPstr << " " << curr_attach.port);
        multi_server = multicast_server(attachedInterface, attachedIPstr, curr_attach.port);
        if (multi_server.sock < 0) {
            LOG_ERROR(SinkVITA49_i, "Error: SinkVITA49_impl::RECEIVER() failed to connect to multicast socket server");
            return false;
        }
        multicast_udp_open = true;
        multicast = true;
    } else if (!curr_attach.ip_address.empty()) {
        multicast = false;
        if (curr_attach.use_udp_protocol) {

            LOG_DEBUG(SinkVITA49_i, "Enabling unicast_client on " << attachedInterface << " " << attachedIPstr << " " << curr_attach.port);
            uni_server = unicast_server(attachedInterface, attachedIPstr, curr_attach.port);
            if (uni_server.sock < 0) {
                LOG_ERROR(SinkVITA49_i, "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket server");
                return false;
            }
            unicast_udp_open = true;
        } else {
            tcp_server = unicast_tcp_server(attachedInterface, attachedIPstr, curr_attach.port);
            LOG_DEBUG(SinkVITA49_i, "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket");
            unicast_tcp_open = true;
        }
    }
    runThread = true;

    // Create context thread BEFORE transmit thread so that context packet is the
    // first packet that gets sent
    if (timeOut > 0)
        _contextThread = new boost::thread(&SinkVITA49_i::timerThread, this);
    if (multicast)
        _transmitThread = new boost::thread(&SinkVITA49_i::TRANSMITTER_M, this);
    else {
        _transmitThread = new boost::thread(&SinkVITA49_i::TRANSMITTER, this);
    }
    
    LOG_INFO(SinkVITA49_i, " =============================================================================");
    LOG_INFO(SinkVITA49_i, " ---- TRANSMITTING PACKETS ON '" << curr_attach.eth_dev << "' AT " << curr_attach.ip_address << ":" << curr_attach.port);
    LOG_INFO(SinkVITA49_i, " =============================================================================");
    return true;
}

//create a context packet every X seconds

void SinkVITA49_i::timerThread() {
    long sleepAmount = (timeOut*1e6)/10;
    while (runThread) {
        if (!waitingForSRI) {
            createIFContextPacket(_tContext, 0);
            boost::this_thread::interruption_point();
        }
        for (int i =0; i<10;i++) {
            if (not(runThread))
                break;
            usleep(sleepAmount);
        }
    }
}

void SinkVITA49_i::TRANSMITTER_M() {
    BasicVRTPacket *vrtPacket;
    BasicVRLFrame *vrl_frame = new BasicVRLFrame();
    int frameCounter = 0;
    long pCount = 0;
    while (runThread) {
        boost::this_thread::interruption_point();
        if (!workQueue2.empty()) {
            {
                if (_throttleTime > 0 && burstPacketCount == pCount) {
                    pCount = 0;
                    usleep(_throttleTime);
                }
                boost::mutex::scoped_lock lock(workQueueLock);
                vrtPacket = workQueue2.front();
                if (VITAProcess.Encap.enable_vrl_frames) {
                    vrl_frame->setVRTPackets(*vrtPacket);
                    if (VITAProcess.Encap.enable_crc)
                        vrl_frame->updateCRC();
                    vrl_frame->setFrameCount((frameCounter++) & 0xFFF);
                    multicast_transmit(multi_server, vrl_frame->getFramePointer(), vrl_frame->getFrameLength());
                    pCount++;
                } else {
                    multicast_transmit(multi_server, vrtPacket->getPacketPointer(), vrtPacket->getPacketLength());
                    pCount++;
                }
                workQueue2.pop();
            }
            delete vrtPacket;

        } else {
            usleep(1e5);
        }
        boost::this_thread::interruption_point();
    }
}

void SinkVITA49_i::TRANSMITTER() {
    BasicVRTPacket *vrtPacket;
    BasicVRLFrame *vrl_frame = new BasicVRLFrame();
    int frameCounter = 0;
    long pCount = 0;
    int result;
    unicast_tcp_t client;
    bool firstPacket = true;

    // TODO: Right now this only accepts one connection.
    // Should be replaced with the boost version borrowed
    // from the Sink and Source Socket
    if (unicast_tcp_open) {
        client = unicast_tcp_accept(tcp_server);
    }

    while (runThread) {
        boost::this_thread::interruption_point();
       
        if (!workQueue2.empty()) {
            {
                if (_throttleTime > 0 && burstPacketCount == pCount) {
                    pCount = 0;
                    usleep(_throttleTime);
                }
                boost::mutex::scoped_lock lock(workQueueLock);
                vrtPacket = workQueue2.front();
                if (VITAProcess.Encap.enable_vrl_frames) {
                    vrl_frame->setVRTPackets(*vrtPacket);

                    if (VITAProcess.Encap.enable_crc)
                        vrl_frame->updateCRC();

                    vrl_frame->setFrameCount((frameCounter++) & 0xFFF);
                    
                    if (unicast_udp_open) {
                        result = unicast_transmit(uni_server, vrl_frame->getFramePointer(), vrl_frame->getFrameLength());
                        LOG_DEBUG(SinkVITA49_i, "Transmitted UDP data..." << result << strerror(errno));
                    }

                    if (unicast_tcp_open) {
                        result = unicast_tcp_transmit(client, vrl_frame->getFramePointer(), vrl_frame->getFrameLength());
                        LOG_DEBUG(SinkVITA49_i, "Transmitted TCP data..." << result << strerror(errno));
                    }
                } else {
                    if (unicast_udp_open) {
                        result = unicast_transmit(uni_server, vrtPacket->getPacketPointer(), vrtPacket->getPacketLength());
                        LOG_DEBUG(SinkVITA49_i, "Transmitted UDP data..." << result << strerror(errno));
                    }

                    if (unicast_tcp_open) {
                        result = unicast_tcp_transmit(client, vrtPacket->getPacketPointer(), vrtPacket->getPacketLength());
                        LOG_DEBUG(SinkVITA49_i, "Transmitted TCP data..." << result << strerror(errno));

                        if (firstPacket && vrtPacket->getPacketType() == PacketType_Data) {
                            firstPacket = false;
                        }
                    }
                }
                workQueue2.pop();
            }
            delete vrtPacket;
        } else {
            usleep(1e5);
        }
        boost::this_thread::interruption_point();
    }
}

int SinkVITA49_i::createPayload(int size, bool signed_v) {
    try {
        int bytesPerPacket = 0;
        int difference = 0;
        in_addr_t attachedIP = inet_network(curr_attach.ip_address.c_str());

        // UDP Header
        if (curr_attach.use_udp_protocol || (attachedIP > lowMulti && attachedIP < highMulti && not curr_attach.ip_address.empty())) {
            bytesPerPacket += UDP_HEADER_SIZE;
        }
        // TCP Header
        else {
            bytesPerPacket += TCP_HEADER_SIZE;
        }

        // VRL Frame
        if (VITAProcess.Encap.enable_vrl_frames) {
            bytesPerPacket += VRL_FRAME_SIZE;
        }

        if (VITAProcess.Encap.enable_crc) {
            bytesPerPacket += VRL_CRC_SIZE;
        }

        // VRT IF Data
        if (VITAProcess.IFDPacket.enable) {
            bytesPerPacket += VITA49_HEADER_SIZE;

            if (VITAProcess.IFDPacket.enable_stream_identifier) {
                bytesPerPacket += VITA49_STREAM_ID_SIZE;
            }

            if (VITAProcess.IFDPacket.enable_class_identifier) {
                bytesPerPacket += VITA49_CLASS_ID_SIZE;
            }

            if (VITAProcess.IFDPacket.embed_time_stamp) {
                bytesPerPacket += VITA49_INT_SECS_SIZE;
                bytesPerPacket += VITA49_FRAC_SECS_SIZE;
            }

            bytesPerPacket += vita49_payload_size;

            if (VITAProcess.IFDPacket.enable_trailer) {
                bytesPerPacket += VITA49_TRAILER_SIZE;
            }
        }

        if (bytesPerPacket > 65515) {
            //find the difference
            difference = bytesPerPacket - 65515;
            bytesPerPacket = 65515;
        }

        connection_status.packet_size = bytesPerPacket;

        RealComplexType type;
        if (currSRI.mode == 0)
            type = RealComplexType_Real;
        else
            type = RealComplexType_ComplexCartesian;

        pf->setRealComplexType(type);

        DataType dType;
        DataItemFormat format;

        int fieldSize = 0;

        if (signed_v) {
            if (size == 1)
                dType = DataType_Int8;
            else
                dType = DataType_Int16;
            format = DataItemFormat_SignedInt;
        } else {
            if (size == 1)
                dType = DataType_UInt8;
            else
                dType = DataType_UInt16;
            format = DataItemFormat_UnsignedInt;
        }
        fieldSize = size * 8;

        if (size == 4) {
            format = DataItemFormat_Float;
            dType = DataType_Float;
        } else if (size == 8) {
            format = DataItemFormat_Double;
            dType = DataType_Double;
        }

        pf->setDataType(dType);
        pf->setDataItemFormat(format);

        if (difference > 0) {
            int subtract_bytes = (difference - ((1 * currSRI.mode + 1) * size)) - 1 - (difference - 1) % ((1 * currSRI.mode + 1) * size);
            vita49_payload_size -= subtract_bytes;
        }
        samplesPerPacket = floor(vita49_payload_size / ((1 * currSRI.mode + 1) * size));

        //add set classid here
        //create the classid assuming we are using a standardPacket
        StandardDataPacket *newP = new StandardDataPacket();

        newP->setPayloadFormat(pf->getBits());
        standardPacketClassID = newP->getClassID();
        
        //std::cout << "Bits: " << pf->getBits() << std::endl;
        //int64_t itmSz = pf->getBits() & __INT64_C(0x0000001F00000000);
        //int64_t fmtSz = pf->getBits() & __INT64_C(0x00000FE000000000);
        //std::cout << "IsValid?: " << pf->getValid() << std::endl;
        //std::cout << "PfInfo: " << pf->toString() << std::endl;
        //std::cout << "FieldSize: " << fieldSize << std::endl;
        //std::cout << "DataType : " << dType << std::endl;
        //std::cout << "Format   : " << format << std::endl;
        //std::cout << "PackField: " << fieldSize << std::endl;
        //std::cout << "itmSz: " << itmSz << std::endl;
        //std::cout << "fmtSz: " << fmtSz << std::endl;
        //std::cout << "(itmSz << 6): " << (itmSz << 6) << std::endl;
        //std::cout << "(itmSz << 6) != fmtSz ::: " << ((itmSz << 6) != fmtSz) << std::endl;
        //std::cout << "PackField: " << fieldSize << std::endl;
        //std::cout << "StandrdPClassId: " << standardPacketClassID << std::endl;

        delete newP;
    } catch (vrt::VRTException &ex) {
       std::cout << "CAUGHT VRT EXCEPTION WHILE CREATING PAYLOAD!: what(): " << ex.what() << std::endl;
    }
    return 1;
}

void SinkVITA49_i::createPacket(BasicDataPacket* pkt, TimeStamp vrt_ts, int sampleIndex_l) {
    //std::string streamID = CORBA::string_dup(currSRI.streamID);
    try {
        if (VITAProcess.IFDPacket.enable) {
            /* use the vector magic to make this work */
            std::_Vector_base<char, _seqVector::seqVectorAllocator<char> >::_Vector_impl *vectorPointer = (std::_Vector_base<char, _seqVector::seqVectorAllocator<char> >::_Vector_impl *) ((void*) & (standardDPacket->bbuf));
            vectorPointer->_M_start = const_cast<char*> (&pkt->bbuf[0]);
            vectorPointer->_M_finish = vectorPointer->_M_start + pkt->bbuf.size();
            vectorPointer->_M_end_of_storage = vectorPointer->_M_finish;

            if (VITAProcess.IFDPacket.enable_stream_identifier) {
                //standardDPacket->setStreamIdentifier(_streamMap[streamID]+streamIDoffset);
                standardDPacket->setStreamIdentifier(_streamMap.hash);
            }

            if (VITAProcess.IFDPacket.enable_class_identifier) {
                //sets the class_identifier and the payload format
                //standardDPacket->setPayloadFormat(pf->getBits());
                standardDPacket->setClassID(standardPacketClassID);
            }

            if (VITAProcess.IFDPacket.embed_time_stamp) {
                standardDPacket->setTimeStamp(vrt_ts);
            }

            standardDPacket->setPacketCount(packetCount % 16);
        }
    } catch (vrt::VRTException &ex) {
       std::cout << "CAUGHT VRT EXCEPTION WHILE CREATING PACKET!: what(): " << ex.what() << std::endl;
    }

    //match the class_identifier of the context packet
    packetCount++;
}

int SinkVITA49_i::createIFContextPacket(BULKIO::PrecisionUTCTime t, int index) {
    TimeStamp ts;
    BasicContextPacket* contextPacket = new BasicContextPacket();
    if (runThread) {
        if (strcmp(_streamMap.streamID.c_str(), currSRI.streamID) != 0) {
            LOG_ERROR(SinkVITA49_i, currSRI.streamID << " Does not Match " << _streamMap.streamID);
            //the stream id was not found in the stream map
            return NOOP;
        }
        ts = calcNextTimeStamp(t, (double) currSRI.xdelta, index);
        //createIFContextPacket(cp,nextTimeStamp);
        bool changed = false;
        /* fill out the packet will all fields per the VITA49 Spec */
        // All fields are persistent unless otherwise marked

/*
        contextPacket->setChangePacket(false);
        contextPacket->setReferencePointIdentifier(0);
        contextPacket->setBandwidth(0.0);
        contextPacket->setFrequencyIF(0.0);
        contextPacket->setFrequencyRF(0.0);
        contextPacket->setFrequencyOffsetRF(0.0);
        contextPacket->setBandOffsetIF(0.0);
        contextPacket->setReferenceLevel(0.0);
        contextPacket->setGain(0.0);
        //contextPacket->setGain1(0.0); //?
        //contextPacket->setGain2(0.0); //?
        contextPacket->setOverRangeCount(0);
        contextPacket->setSampleRate(0.0);
        //contextPacket->setSamplePeriod(0.0); //?
        contextPacket->setTimeStampAdjustment(0);
        contextPacket->setTimeStampCalibration(0);
        contextPacket->setTemperature(0);
        contextPacket->setCalibratedTimeStamp(_FALSE);
        contextPacket->setDataValid(_FALSE);
        contextPacket->setReferenceLocked(_FALSE);
        contextPacket->setAutomaticGainControl(_FALSE);
        contextPacket->setSignalDetected(_FALSE);
        contextPacket->setInvertedSpectrum(_FALSE); //?
        contextPacket->setOverRange(_FALSE); // Not persistent
        contextPacket->setDiscontinuous(_FALSE); //Not persistent
        contextPacket->setDataPayloadFormat(pf->getBits());
        contextPacket->setUserDefinedBits(0); //? Not Persistent
*/

        if (VITAProcess.IFCPacket.enable_stream_identifier)
            contextPacket->setStreamIdentifier(_streamMap.hash + VITAProcess.IFCPacket.stream_identifier_offset); // The stream ID
        if (VITAProcess.IFCPacket.enable_class_identifier) {
            if (strcmp(VITAProcess.IFCPacket.class_identifier.c_str(), "DEFAULT") == 0) {
                contextPacket->setClassID(standardPacketClassID);
                VITAProcess.IFCPacket.class_identifier = standardPacketClassID;
            } else {
                std::string classID(VITAProcess.IFCPacket.class_identifier);
                contextPacket->setClassID(classID);
            }
        }

        if (VITAProcess.IFCPacket.enable_device_identifier) {
            std::string device_identifier(VITAProcess.IFCPacket.device_identifier);
            contextPacket->setDeviceID(device_identifier);
        }

        if (VITAProcess.IFCPacket.embed_time_stamp) {
            contextPacket->setTimeStamp(ts);
        }
        contextPacket->setPacketCount(contextCount & 0xF);
        contextCount++;
        contextPacket->setDataPayloadFormat(pf->getBits());
        contextPacket->setSampleRate(1.0 / currSRI.xdelta);
        
        double value_d;
        float value_f;
        long value_l;
        bool value_b;

        unsigned long currSize = currSRI.keywords.length();
        for (unsigned long i = 0; i < currSize; ++i) {
            if (strcmp("COL_BW", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;

                contextPacket->setBandwidth(value_d);
                changed = true;
            } else if (strcmp("COL_IF_FREQUENCY", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setFrequencyIF(value_d);
                changed = true;
            } else if (strcmp("COL_RF", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setFrequencyRF(value_d);
                changed = true;
            } else if (strcmp("COL_RF_OFFSET", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setFrequencyOffsetRF(value_d);
                changed = true;
            } else if (strcmp("COL_IF_FREQUENCY_OFFSET", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setBandOffsetIF(value_d);
                changed = true;
            } else if (strcmp("COL_REFERENCE_LEVEL", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_f;
                contextPacket->setReferenceLevel(value_f);
                changed = true;
            } else if (strcmp("REFERENCE_POINT_IDENTIFIER", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_l;
                contextPacket->setReferencePointIdentifier(value_l);
                changed = true;
            } else if (strcmp("COL_GAIN", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_f;
                contextPacket->setGain1(value_f);
                changed = true;
            } else if (strcmp("DATA_GAIN", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_f;
                contextPacket->setGain2(value_f);
                changed = true;
            } else if (strcmp("OVER_RANGE_SUM", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setOverRangeCount((long long) value_d);
                changed = true;
            } else if (strcmp("USER_DEFINED", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_l;
                contextPacket->setUserDefinedBits((int32_t) value_l);
                changed = true;
            } else if (strcmp("TIMESTAMP_ADJUSTMENT_PICOSECONDS", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_d;
                contextPacket->setTimeStampAdjustment((long long) value_d);
                changed = true;
            } else if (strcmp("TIMESTAMP_CALIBRATION", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_l;
                contextPacket->setTimeStampCalibration((int32_t) value_l);
                changed = true;
            } else if (strcmp("TEMPERATURE", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_f;
                contextPacket->setTemperature(value_f);
                changed = true;
            } else if (strcmp("DATA_VALID", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setDataValid(_TRUE);
                if (!value_b)
                    contextPacket->setDataValid(_FALSE);
                changed = true;
            } else if (strcmp("REFERENCE_LOCKED", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setReferenceLocked(_TRUE);
                if (!value_b)
                    contextPacket->setReferenceLocked(_FALSE);
                changed = true;
            } else if (strcmp("CALIBRATED_TIME_STAMP", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setCalibratedTimeStamp(_TRUE);
                if (!value_b)
                    contextPacket->setCalibratedTimeStamp(_FALSE);
                changed = true;
            } else if (strcmp("AUTO_GAIN_CONTROL", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setAutomaticGainControl(_TRUE);
                if (!value_b)
                    contextPacket->setAutomaticGainControl(_FALSE);
                changed = true;
            } else if (strcmp("SIGNAL_DETECTION", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setSignalDetected(_TRUE);
                if (!value_b)
                    contextPacket->setSignalDetected(_FALSE);
                changed = true;
            } else if (strcmp("DATA_INVERSION", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setInvertedSpectrum(_TRUE);
                if (!value_b)
                    contextPacket->setInvertedSpectrum(_FALSE);
                changed = true;
            } else if (strcmp("OVER_RANGE", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setOverRange(_TRUE);
                if (!value_b)
                    contextPacket->setOverRange(_FALSE);
                changed = true;
            } else if (strcmp("SAMPLE_LOSS", currSRI.keywords[i].id) == 0) {
                currSRI.keywords[i].value >>= value_b;
                if (value_b)
                    contextPacket->setDiscontinuous(_TRUE);
                if (!value_b)
                    contextPacket->setDiscontinuous(_FALSE);
                changed = true;
            }
            else if (strcmp("GEOLOCATION_GPS", currSRI.keywords[i].id) == 0) {
                GEOLOCATION_GPS_struct geolocation_gps;
                currSRI.keywords[i].value >>= geolocation_gps;
                TimeStamp *time = new TimeStamp(IntegerMode_GPS, geolocation_gps.TIME_SECONDS, geolocation_gps.TIME_FRACTIONAL * 1e9, 1.0 / currSRI.xdelta);
                processingGeolocation.setTimeStamp(*time);
                processingGeolocation.setAltitude(geolocation_gps.ALTITUDE);
                processingGeolocation.setHeadingAngle(geolocation_gps.HEADING_ANGLE);
                processingGeolocation.setLatitude(geolocation_gps.LATITUDE);
                processingGeolocation.setLongitude(geolocation_gps.LONGITUDE);
                processingGeolocation.setMagneticVariation(geolocation_gps.MAGNETIC_VARIATION);
                processingGeolocation.setManufacturerIdentifier(geolocation_gps.MANUFACTURER_ID);
                processingGeolocation.setSpeedOverGround(geolocation_gps.GROUND_SPEED);
                processingGeolocation.setTrackAngle(geolocation_gps.TRACK_ANGLE);

                contextPacket->setGeolocationGPS(processingGeolocation);
                delete time;
            }
            else if (strcmp("GEOLOCATION_INS", currSRI.keywords[i].id) == 0) {
                GEOLOCATION_INS_struct geolocation_ins;
                currSRI.keywords[i].value >>= geolocation_ins;
                TimeStamp *time = new TimeStamp(IntegerMode_GPS, geolocation_ins.TIME_SECONDS, geolocation_ins.TIME_FRACTIONAL * 1e9, 1.0 / currSRI.xdelta);
                processingGEOINS.setTimeStamp(*time);
                processingGEOINS.setAltitude(geolocation_ins.ALTITUDE);
                processingGEOINS.setHeadingAngle(geolocation_ins.HEADING_ANGLE);
                processingGEOINS.setLatitude(geolocation_ins.LATITUDE);
                processingGEOINS.setLongitude(geolocation_ins.LONGITUDE);
                processingGEOINS.setMagneticVariation(geolocation_ins.MAGNETIC_VARIATION);
                processingGEOINS.setManufacturerIdentifier(geolocation_ins.MANUFACTURER_ID);
                processingGEOINS.setSpeedOverGround(geolocation_ins.GROUND_SPEED);
                processingGEOINS.setTrackAngle(geolocation_ins.TRACK_ANGLE);

                contextPacket->setGeolocationINS(processingGEOINS);
                delete time;
            }
            else if (strcmp("EPHEMERIS_ECEF", currSRI.keywords[i].id) == 0) {
                EPHEMERIS_ECEF_struct ephemeris_ecef;
                currSRI.keywords[i].value >>= ephemeris_ecef;
                TimeStamp *time = new TimeStamp(IntegerMode_GPS, ephemeris_ecef.TIME_SECONDS, ephemeris_ecef.TIME_FRACTIONAL_SECONDS * 1e9, 1.0 / currSRI.xdelta);
                processingEphemeris.setTimeStamp(*time);
                processingEphemeris.setPositionX(ephemeris_ecef.POSITION_X);
                processingEphemeris.setPositionY(ephemeris_ecef.POSITION_Y);
                processingEphemeris.setPositionZ(ephemeris_ecef.POSITION_Z);
                processingEphemeris.setAttitudeAlpha(ephemeris_ecef.ATTITUDE_ALPHA);
                processingEphemeris.setAttitudeBeta(ephemeris_ecef.ATTITUDE_BETA);
                processingEphemeris.setAttitudePhi(ephemeris_ecef.ATTITUDE_PHI);
                processingEphemeris.setVelocityX(ephemeris_ecef.VELOCITY_X);
                processingEphemeris.setVelocityY(ephemeris_ecef.VELOCITY_Y);
                processingEphemeris.setVelocityZ(ephemeris_ecef.VELOCITY_Z);
                delete time;

                contextPacket->setEphemerisECEF(processingEphemeris);
            }
            else if (strcmp("EPHEMERIS_RELATIVE", currSRI.keywords[i].id) == 0) {
                EPHEMERIS_RELATIVE_struct ephemeris_relative;
                currSRI.keywords[i].value >>= ephemeris_relative;
                TimeStamp *time = new TimeStamp(IntegerMode_GPS, ephemeris_relative.TIME_SECONDS, ephemeris_relative.TIME_FRACTIONAL_SECONDS * 1e9, 1.0 / currSRI.xdelta);
                processingEphemerisRel.setTimeStamp(*time);
                processingEphemerisRel.setPositionX(ephemeris_relative.POSITION_X);
                processingEphemerisRel.setPositionY(ephemeris_relative.POSITION_Y);
                processingEphemerisRel.setPositionZ(ephemeris_relative.POSITION_Z);
                processingEphemerisRel.setAttitudeAlpha(ephemeris_relative.ATTITUDE_ALPHA);
                processingEphemerisRel.setAttitudeBeta(ephemeris_relative.ATTITUDE_BETA);
                processingEphemerisRel.setAttitudePhi(ephemeris_relative.ATTITUDE_PHI);
                processingEphemerisRel.setVelocityX(ephemeris_relative.VELOCITY_X);
                processingEphemerisRel.setVelocityY(ephemeris_relative.VELOCITY_Y);
                processingEphemerisRel.setVelocityZ(ephemeris_relative.VELOCITY_Z);
                delete time;
                //contextPacket->setEphemerisRelative(processingEphemerisRel);
            }
        }
        
        contextPacket->setChangePacket(changed);
        {
            boost::mutex::scoped_lock lock(workQueueLock);
            workQueue2.push(contextPacket);
        }
    }
    return NORMAL;
}

/********************************************************************************************
 * serviceFunction()
 *
 * Takes:   void
 * Returns: int
 *
 * Functionality:
 *   The service function is called by the serviceThread object (of type ProcessThread).
 *   This call happens immediately after the previous call if the return value for
 *   the previous call was NORMAL.
 *   If the return value for the previous call was NOOP, then the serviceThread waits
 *   an amount of time defined in the serviceThread's constructor.
 *
 *   This function calls the singleService function for each of the input ports and
 *   returns NORMAL if one or more of those ports received and processed data. 
 *********************************************************************************************/
int SinkVITA49_i::serviceFunction() {
    bool retService;
    if (createMem) {
        memoryManagement(vita49_payload_size);
    }
    if (shouldUpdateStream) {
        destroy_tx_thread();
        shouldUpdateStream = false;
    }
    if (_transmitThread == NULL and ((numberOutputConnections() > 0) or advanced_configuration.force_transmit)) {
        updateCurrAttach();
        launch_tx_thread();
    }
    //assumes only one provides port is active at a time
    retService = singleService(dataDouble_in, false);
    retService = retService || singleService(dataFloat_in, false);
    retService = retService || singleService(dataUshort_in, false);
    retService = retService || singleService(dataShort_in, true);
    retService = retService || singleService(dataChar_in, true);
    retService = retService || singleService(dataOctet_in, false);

    if (retService) {
        return NORMAL;
    }

    return NOOP;
}

/***********************************************************************************************
 * singleService()
 *
 * Takes:   IN *dataIn -> pointer to an input port
 * Returns: bool
 *
 * Functionality:
 *   Data is passed to the singleService function through the getPacket call (BULKIO only).
 *   The dataTransfer class is a port-specific class, so each port implementing the
 *   BULKIO interface will have its own type-specific dataTransfer.
 *
 *   The argument to the getPacket function is a floating point number that specifies
 *   the time to wait in seconds. A zero value is non-blocking. A negative value
 *   is blocking.
 *
 *   Each received dataTransfer is owned by serviceFunction and *MUST* be
 *   explicitly deallocated.
 *
 *   This function takes care of getting in the data from some source.  It then pulls the necessary
 *   number of packets out of the VITA49 bank one at time, populates the packet, and pushes it into
 *   the work queue. 
 *************************************************************************************************/

bool SinkVITA49_i::readyToProcessPacket(const std::string incomingStreamId) {    
    // Force packet process
    if (advanced_configuration.force_transmit) {
        return true;
    }
    
    bool hasActiveStream = hasActiveOutputStream();
    
    // No reason to process packet if there aren't any listeners
    if (numberOutputConnections() == 0 ) {
        if (hasActiveStream) {
            // Tear down an active stream (incase output is disconnected runtime)
            LOG_INFO(SinkVITA49_i, "SinkVITA49_out stream '" << _streamMap.streamID << "' interrupted!");
            tearDownOutputStream();
        }
        LOG_DEBUG(SinkVITA49_i, "NO LISTENERS...NOT SENDING CONVERTING/SENDING PACKETS...")
        return false;
    }
    
    // Filter out packets unrelated to active stream
    if (hasActiveStream && _streamMap.streamID != incomingStreamId) {
        LOG_WARN(SinkVITA49_i, "SinkVITA49 can not handle multiple streams on input, streamID:" << incomingStreamId << " ");
        LOG_WARN(SinkVITA49_i, "Dropping packet for streamID:'" << incomingStreamId << "' since it is not part of streamID:'" << _streamMap.streamID << "'");
        return false;
    }
    
    return true;
}

template <class IN> bool SinkVITA49_i::singleService(IN *dataIn, bool signedPort) {
    LOG_TRACE(SinkVITA49_i, __PRETTY_FUNCTION__);
    
    // Grab a packet and return if no packet exists in queue
    typename IN::dataTransfer *CORBApacket = dataIn->getPacket(0);
    if (CORBApacket == NULL)
        return NOOP;
      
    // Setup processing parameters
    std::string incomingStreamId = CORBApacket->streamID;
    BasicDataPacket *vrtPacket = NULL;
    int complexMultiplier = currSRI.mode + 1;
    int sampleSize = sizeof (CORBApacket->dataBuffer.front());
    int leftOverSamples = (leftOverDataSize / sampleSize) / (1 * complexMultiplier);
    int numSamples = (CORBApacket->dataBuffer.size() / complexMultiplier) + leftOverSamples;
    int dataSizeInBytes = numSamples*sampleSize *(1+currSRI.mode);
    
    // Validate that we can process received packet
    if (not readyToProcessPacket(incomingStreamId)) {
        LOG_DEBUG(SinkVITA49_i, "DROPPING PACKET...")
        delete CORBApacket;
        return NOOP;
    }
    
    // First packet defines the stream information
    if (not hasActiveOutputStream()) {
        LOG_DEBUG(SinkVITA49_i, "SETTING UP STREAM '" << incomingStreamId << "'");
        setupOutputStream(incomingStreamId, sampleSize, signedPort);
    }
    
    // Handle SRI changes
    if (curr_attach.attach && (CORBApacket->sriChanged || waitingForSRI)) {
        bool t = mergeRecSRI(CORBApacket->SRI, CORBApacket->T);
        if (t && VITAProcess.IFCPacket.enable) {
            createPayload(sampleSize, signedPort);
            //nextTimeStamp = calcNextTimeStamp(CORBApacket->T,currSRI.xdelta,dataIndex);
            if (leftOverDataSize == 0)
                createIFContextPacket(CORBApacket->T, 0);
            else
                createIFContextPacket(CORBApacket->T, -leftOverSamples);
        }
        waitingForSRI = false;
    }
    
    // Validate that we can process received packet
    if (samplesPerPacket < 1) {
        LOG_DEBUG(SinkVITA49_i, "Property 'samplesPerPacket' must be greater than 0!: DROPPING PACKET...")
        delete CORBApacket;
        return NOOP;
    }
    
    // Create VRT packets and push them to the queue
    while (dataSizeInBytes >= (samplesPerPacket * complexMultiplier * sampleSize)) {
        try {
            vrtPacket = new BasicDataPacket();
            vrtPacket->setPayloadFormat(pf->getBits());
            //PayloadFormat pf_l = vrtPacket->getPayloadFormat();
            vrtPacket->setPayloadLength(samplesPerPacket * ((1 * currSRI.mode + 1) * sampleSize));

            nextTimeStamp = calcNextTimeStamp(CORBApacket->T, (double) currSRI.xdelta, dataIndex/(1*currSRI.mode + 1));
            createPacket(vrtPacket, nextTimeStamp, dataIndex);
            if (VITAProcess.IFDPacket.enable_trailer) {
                vrtPacket->setAssocPacketCount(contextCount & 0x7F);
            }

            if (leftOverDataSize > 0) {
                //memcpy(spareBuffer,leftoverCORBApacket->dataBuffer.data()+(leftoverCORBApacket->dataBuffer.size()-leftOverDataSize),leftOverDataSize*sampleSize);
                memcpy(&spareBuffer[leftOverDataSize], CORBApacket->dataBuffer.data(), (samplesPerPacket * ((1 * currSRI.mode + 1) * sampleSize)) - leftOverDataSize);
                nextTimeStamp = calcNextTimeStamp(CORBApacket->T, (double) currSRI.xdelta, -(leftOverDataSize / sampleSize) / (1 * currSRI.mode + 1));
                vrtPacket->setTimeStamp(nextTimeStamp);
                vrtPacket->setData(pf->getBits(), spareBuffer, samplesPerPacket * ((1 * currSRI.mode + 1) * sampleSize), convertEndian);
                dataIndex += (samplesPerPacket * (1 * currSRI.mode + 1))-(leftOverDataSize / sampleSize);
                leftOverDataSize = 0;
            } else {
                //nextTimeStamp = calcNextTimeStamp(CORBApacket->T,currSRI.xdelta,dataIndex);

                vrtPacket->setData(pf->getBits(), &CORBApacket->dataBuffer[dataIndex], samplesPerPacket * ((1 * currSRI.mode + 1) * sampleSize), convertEndian);
                dataIndex += (samplesPerPacket * (1 * currSRI.mode + 1));
            }
            dataSizeInBytes -= (samplesPerPacket * ((1 * currSRI.mode + 1) * sampleSize));
            //update the context packet time
            _tContext.tfsec = nextTimeStamp.getFractionalSeconds() / 10e9;
            _tContext.twsec = nextTimeStamp.getUTCSeconds();
            //LOG_DEBUG(SinkVITA49_i, __PRETTY_FUNCTION__ << " ABOUT TO PUSH TO QUEUE ");
            {
                boost::mutex::scoped_lock lock(workQueueLock);
                workQueue2.push(vrtPacket);
            }
        } catch (vrt::VRTException &ex) {
           std::cout << "CAUGHT VRT EXCEPTION!: what(): " << ex.what() << std::endl;
        }
    }

    // Save off remainder of data that doesn't fill an entire packet
    if (dataSizeInBytes > 0) {
        // EdgeCase: 
        //   1.) If there are left over packets
        //   2.) If the next packet coming in still doesn't have enough data to make
        //       a pushPacket call
        //   * Don't
        //
        if ((&CORBApacket->dataBuffer)->size() > 0) {
          memcpy(&spareBuffer[0], &CORBApacket->dataBuffer[dataIndex], dataSizeInBytes);
          leftOverDataSize = dataSizeInBytes; //in bytes
        }
    }

    // Push remainder of data if we reached end of stream
    if (CORBApacket->EOS) {
        LOG_DEBUG(SinkVITA49_i, "ServiceFunction: Received EOS for stream '" << CORBApacket->streamID << "'")
        try {
            if (dataSizeInBytes > 0) {
                vrtPacket = new BasicDataPacket();
                vrtPacket->setPayloadFormat(pf->getBits());
                vrtPacket->setPayloadLength(dataSizeInBytes);
                nextTimeStamp = calcNextTimeStamp(CORBApacket->T, (double) currSRI.xdelta, dataIndex/(1*currSRI.mode+1));
                createPacket(vrtPacket, nextTimeStamp, dataIndex);

                vrtPacket->setData(pf->getBits(), &spareBuffer[0], dataSizeInBytes, convertEndian);
                {
                    boost::mutex::scoped_lock lock(workQueueLock);
                    workQueue2.push(vrtPacket);
                }
            }
        } catch (vrt::VRTException &ex) {
           std::cout << "CAUGHT VRT EXCEPTION!: what(): " << ex.what() << std::endl;
        }
        tearDownOutputStream();
    }

    dataIndex = 0;

    /* delete the dataTransfer object */
    delete CORBApacket;

    return true;
}

void SinkVITA49_i::setupOutputStream(const std::string streamID, int sampleSize, bool signedPort) {
    //we need a hash of the string to create the stream ID for the context packet
    boost::hash<std::string> string_hash;
    string_hash(streamID);
    _streamMap.hash = (unsigned int) string_hash(streamID);
    _streamMap.streamID = streamID;
    initstreamDef(sampleSize, signedPort);

    addModifyKeyword<long>(&currSRI, "dataRef", _dataRef);
    addModifyKeyword<bool>(&currSRI, "BULKIO_SRI_PRIORITY", _bulkioPriority);

    _streamDef.id = CORBA::string_dup(streamID.c_str());
    printStreamDef(_streamDef);
    dataVITA49_out->addStream(_streamDef);
    curr_attach.attach = true;
}

void SinkVITA49_i::tearDownOutputStream() {
  try {
    dataVITA49_out->removeStream(_streamMap.streamID.c_str());
    resetStreamMap();
    destroy_tx_thread(); // TODO: Why is this needed?
    initialize_values();
  } catch (...) {
    LOG_ERROR(SinkVITA49_i, "TODO: Fix issue with tearDownOutputStream");
  };
}

bool SinkVITA49_i::hasActiveOutputStream() {
    return (_streamMap.hash != 0);
}

int SinkVITA49_i::numberOutputConnections() {
    ExtendedCF::UsesConnectionSequence* connections = dataVITA49_out->connections();
    size_t connCount = connections->length();
    delete connections;
    return connCount;
}

void SinkVITA49_i::updateStreamDef() {
    _streamDef.vlan = curr_attach.vlan;
    _streamDef.port = curr_attach.port;
    _streamDef.ip_address = CORBA::string_dup(curr_attach.ip_address.c_str());
    if (curr_attach.use_udp_protocol)
        _streamDef.protocol = BULKIO::VITA49_UDP_TRANSPORT;
    else
        _streamDef.protocol = BULKIO::VITA49_TCP_TRANSPORT;
    
    // If running attachments
    BULKIO::VITA49StreamSequence* attachedStreams = this->dataVITA49_out->attachedStreams();
    if (attachedStreams->length() > 0) {
        try {
          this->dataVITA49_out->updateStream(_streamDef);
        } catch(...) {
           LOG_ERROR(SinkVITA49_i, "TODO: Fix updateStream exception handling!")
        }
    }
    delete attachedStreams;
}

void SinkVITA49_i::initstreamDef(int sampleSize, bool signedPort) {
    //always required info
    _streamDef.vlan = curr_attach.vlan;
    _streamDef.port = curr_attach.port;
    _streamDef.ip_address = CORBA::string_dup(curr_attach.ip_address.c_str());
    if (curr_attach.use_udp_protocol)
        _streamDef.protocol = BULKIO::VITA49_UDP_TRANSPORT;
    else
        _streamDef.protocol = BULKIO::VITA49_TCP_TRANSPORT;

    //This is not an override. We include this if and only if there is no context packet
    //in the stream. The information below is required to process the IFDataPacket
    if (!VITA49IFContextPacket.enable) {
        _streamDef.valid_data_format = true;
        _streamDef.data_format.packing_method_processing_efficient = true;

        if (currSRI.mode == 1)
            _streamDef.data_format.complexity = BULKIO::VITA49_COMPLEX_CARTESIAN;
        else
            _streamDef.data_format.complexity = BULKIO::VITA49_REAL;

        if (sampleSize == 4) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_32F;
            _streamDef.data_format.item_packing_field_size = 32;
            _streamDef.data_format.data_item_size = 32;
        } else if (sampleSize == 8) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_64F;
            _streamDef.data_format.item_packing_field_size = 64;
            _streamDef.data_format.data_item_size = 64;
        }

        if (signedPort && sampleSize == 2) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_16T;
            _streamDef.data_format.item_packing_field_size = 16;
            _streamDef.data_format.data_item_size = 16;
        } else if (!signedPort && sampleSize == 2) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_16U;
            _streamDef.data_format.item_packing_field_size = 16;
            _streamDef.data_format.data_item_size = 16;
        } else if (signedPort && sampleSize == 1) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_8T;
            _streamDef.data_format.item_packing_field_size = 8;
            _streamDef.data_format.data_item_size = 8;
        } else if (!signedPort && sampleSize == 1) {
            _streamDef.data_format.data_item_format = BULKIO::VITA49_8T;
            _streamDef.data_format.item_packing_field_size = 8;
            _streamDef.data_format.data_item_size = 8;
        }
        _streamDef.data_format.repeat_count = 1;
        _streamDef.data_format.vector_size = _streamDef.data_format.data_item_size - 1;

    } else {
        //There is a context packet present, tell the attach call to wait for the context packet for the
        //information necessary to decode the IFDatapacket
        _streamDef.valid_data_format = false;
    }
}

void SinkVITA49_i::setDefaultSRI() {
    boost::mutex::scoped_lock lock(sriLock);
    /* "distance" between samples (inverse of sample rate) */
    currSRI.xdelta = (double) 1;

    /* 0 for Scalar, 1 for Complex */
    currSRI.mode = (short) 0;

    currSRI.streamID = "DEFAULT_SINKVITA49_STREAMID";
    currSRI.blocking = false;
    currSRI.hversion = (long) 0;
    currSRI.xstart = (double) 0;

    /* Platinum time code (1 == seconds) */
    currSRI.xunits = (short) 1;

    /* # frames to be delivered by pushPacket() call; set to 0 for single packet */
    currSRI.ystart = (double) 0;
    currSRI.ydelta = (double) 0.001;
    currSRI.yunits = (short) 1;
    currSRI.subsize = 0;
    currSRI.keywords.length(0);
}

bool SinkVITA49_i::compareSRI(BULKIO::StreamSRI A, BULKIO::StreamSRI B) {
    bool same = false;

    if ((A.hversion == B.hversion) and
            (A.xstart == B.xstart) and
            (A.xdelta == B.xdelta) and
            (A.xunits == B.xunits) and
            (A.subsize == B.subsize) and
            (A.ystart == B.ystart) and
            (A.ydelta == B.ydelta) and
            (A.mode == B.mode) and
            (!strcmp(A.streamID, B.streamID))) {
        same = true;
    } else {
        same = false;
        return same;
    }

    if (A.keywords.length() != B.keywords.length()) {
        same = false;
        return same;
    }

    for (unsigned int i = 0; i < A.keywords.length(); i++) {
        string action = "ne";
        if (ossie::compare_anys(A.keywords[i].value, B.keywords[i].value, action)) {
            same = false;
            return same;
        }

    }

    return same;
}

bool SinkVITA49_i::mergeRecSRI(BULKIO::StreamSRI &recSRI, BULKIO::PrecisionUTCTime time) {
    bool updateSRI = false;
    boost::mutex::scoped_lock lock(sriLock);
    if (!compareSRI(recSRI, currSRI)) {
        currSRI.xdelta = recSRI.xdelta;
        currSRI.streamID = recSRI.streamID;
        currSRI.hversion = recSRI.hversion;
        currSRI.xstart = recSRI.xstart;
        currSRI.xunits = recSRI.xunits;
        currSRI.ystart = recSRI.ystart;
        currSRI.ydelta = recSRI.ydelta;
        currSRI.yunits = recSRI.yunits;
        currSRI.mode = recSRI.mode;
        unsigned long keySize = recSRI.keywords.length();
        unsigned long currSize = currSRI.keywords.length();
        updateSRI = true;
        //search through currSRI for the keywords in recSRI
        //this should be the first case....

        for (unsigned long i = 0; i < keySize; ++i) {
            //want to search all of currSize
            std::string action = "eq";
            //look for the id and set to true of found
            bool foundID = false;
            for (unsigned long j = 0; j < currSize; ++j) {
                if (strcmp(recSRI.keywords[i].id, currSRI.keywords[j].id) == 0) {
                    foundID = true;
                    if (!ossie::compare_anys(recSRI.keywords[i].value, currSRI.keywords[j].value, action)) {
                        updateSRI = true;
                        currSRI.keywords[j].value = recSRI.keywords[i].value;
                    }
                }
            }
            //the id was not found we need to extend the length of keywords in currSRI and add this one to the end
            if (!foundID) {
                updateSRI = true;
                unsigned long keySize_t = currSRI.keywords.length();
                currSRI.keywords.length(keySize_t + 1);
                currSRI.keywords[keySize_t].id = CORBA::string_dup(recSRI.keywords[i].id);
                currSRI.keywords[keySize_t].value = recSRI.keywords[i].value;
            }
        }

        if (updateSRI) {
            dataVITA49_out->pushSRI(currSRI, time);
        }
    }
    return updateSRI;
}

void SinkVITA49_i::printSRI(BULKIO::StreamSRI *sri, std::string strHeader) {
    LOG_DEBUG(SinkVITA49_i, strHeader << ":\n");
    LOG_DEBUG(SinkVITA49_i, "\thversion: " << sri->hversion);
    LOG_DEBUG(SinkVITA49_i, "\txstart: " << sri->xstart);
    LOG_DEBUG(SinkVITA49_i, "\txdelta: " << sri->xdelta);
    LOG_DEBUG(SinkVITA49_i, "\txunits: " << sri->xunits);
    LOG_DEBUG(SinkVITA49_i, "\tsubsize: " <<sri->subsize);
    LOG_DEBUG(SinkVITA49_i, "\tystart: " << sri->ystart);
    LOG_DEBUG(SinkVITA49_i, "\tydelta: " << sri->ydelta);
    LOG_DEBUG(SinkVITA49_i, "\tyunits: " << sri->yunits);
    LOG_DEBUG(SinkVITA49_i, "\tmode: " << sri->mode);
    LOG_DEBUG(SinkVITA49_i, "\tstreamID: " << sri->streamID);
    for (size_t i = 0; i < sri->keywords.length(); i++) {
        LOG_DEBUG(SinkVITA49_i, "\t KEYWORD KEY/VAL :: " << sri->keywords[i].id << ": " << ossie::any_to_string(sri->keywords[i].value) );
    }
    LOG_DEBUG(SinkVITA49_i, "");
}

void SinkVITA49_i::printStreamDef( const BULKIO::VITA49StreamDefinition& streamDef){
    std::string space = "    ";
    LOG_DEBUG(SinkVITA49_i, " *** STREAM DEFINITION *** ");
    LOG_DEBUG(SinkVITA49_i, space << "ip_address: " << streamDef.ip_address);
    LOG_DEBUG(SinkVITA49_i, space << "vlan: " << streamDef.vlan);
    LOG_DEBUG(SinkVITA49_i, space << "port: " << streamDef.port);
    LOG_DEBUG(SinkVITA49_i, space << "protocol: " << streamDef.protocol);
    LOG_DEBUG(SinkVITA49_i, space << "valid_data_format: " << streamDef.valid_data_format);
    LOG_DEBUG(SinkVITA49_i, space << "data_format: ");
    LOG_DEBUG(SinkVITA49_i, space << space << "packing_method_processing_efficient: " << streamDef.data_format.packing_method_processing_efficient);
    LOG_DEBUG(SinkVITA49_i, space << space << "complexity: " << streamDef.data_format.complexity);
    LOG_DEBUG(SinkVITA49_i, space << space << "data_item_format: " << streamDef.data_format.data_item_format);
    LOG_DEBUG(SinkVITA49_i, space << space << "repeating: " << streamDef.data_format.repeating);
    LOG_DEBUG(SinkVITA49_i, space << space << "event_tag_size: " << streamDef.data_format.event_tag_size);
    LOG_DEBUG(SinkVITA49_i, space << space << "channel_tag_size: " << streamDef.data_format.channel_tag_size);
    LOG_DEBUG(SinkVITA49_i, space << space << "item_packing_field_size: " << streamDef.data_format.item_packing_field_size);
    LOG_DEBUG(SinkVITA49_i, space << space << "data_item_size: " << streamDef.data_format.data_item_size);
    LOG_DEBUG(SinkVITA49_i, space << space << "repeat_count: " << streamDef.data_format.repeat_count);
    LOG_DEBUG(SinkVITA49_i, space << space << "vector_size: " << streamDef.data_format.vector_size);
    LOG_DEBUG(SinkVITA49_i, "");
};
