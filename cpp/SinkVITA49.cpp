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

 	Source: SinkVITA49.cpp
 	Generated on: Fri Jan 10 11:20:07 EST 2014
 	REDHAWK IDE
 	Version: 1.8.6
 	Build id: N201312130030

*******************************************************************************************/

#include "SinkVITA49.h"

PREPARE_LOGGING(SinkVITA49_i)

using namespace vrt;
/************************************************
 * Constructor
 *
 * Takes:  const char *uuid, const char *label
 ************************************************/
SinkVITA49_i::SinkVITA49_i(const char *uuid, const char *label) :
										SinkVITA49_base(uuid, label){
	//override the output port
	PortableServer::ObjectId_var oid;
	dataVITA49_out = new BULKIO_dataVITA49_Out_implemented("dataVITA49_out", this);
	oid = ossie::corba::RootPOA()->activate_object(dataVITA49_out);
	registerOutPort(dataVITA49_out, dataVITA49_out->_this());

	__constructor__();
}

void SinkVITA49_i::__constructor__(){
	vlan = 0;
	timeOut = 1;
	port = 0;
	_transmitThread = NULL;
	_contextThread = NULL;
	//set ip address range for multicast
	lowMulti = inet_network("224.0.0.1");
	highMulti =  inet_network("239.255.255.250");

	multicast = true;
	waitingForSRI = true;

	createMem = true;
	numBuffers = 200000;
	setDefaultSRI();
	waitForContext = true;
	//contextPacket = NULL;
	tx_thead_running = false;
	pf = new PayloadFormat(true,RealComplexType_ComplexCartesian,DataItemFormat_Double,false,0,0,64,64,1,1);
	//initialize the _streamDef to some settings

	spareBuffer = NULL;

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
	// t(io, boost::posix_time::seconds(5));
	convertEndian = false;
	initialize_values();
	standardDPacket = new StandardDataPacket(100);
	streamIDoffset = 10000;
	_streamMap.hash=0;
	_streamMap.streamID="";
}

void SinkVITA49_i::initialize_values(){
//	_streamMap.clear();
	burstPacketCount = 0;
	_attachMap.clear();
	remainingData = false;
	dataIndex = 0;
	packetCount= 0;
	contextCount = 0;
	samplesPerPacket = 0;
	standardPacketClassID = "";
	leftOverDataSize =0;
	multicast = false;
	//initalize the packet classes
	VITAProcess.IFCPacket.enable = true;
	VITAProcess.IFCPacket.enable_stream_identifier = true;
	//need to reset this according the appendix A
	std::string class_identifier="DEFAULT";
	std::string device_identifier="FF-FF-FA:1301";
	VITAProcess.IFCPacket.enable_class_identifier = true;
	VITAProcess.IFCPacket.enable_device_identifier = true;

	VITAProcess.IFCPacket.class_identifier = CORBA::string_dup(class_identifier.c_str());
	VITAProcess.IFCPacket.device_identifier = CORBA::string_dup(device_identifier.c_str());
	VITAProcess.IFCPacket.embed_time_stamp = true;
	VITAProcess.IFCPacket.use_trailer = false;

	VITAProcess.IFDPacket.enable = true;
	VITAProcess.IFDPacket.enable_stream_identifier = true;
	VITAProcess.IFDPacket.enable_class_identifier = true;
	VITAProcess.IFDPacket.embed_time_stamp = true;
	VITAProcess.IFDPacket.use_trailer = false;
	_tContext.tfsec = 0.0;
	_tContext.twsec = 0.0;
	_dataRef = BYTE_ORDER;
	_throttleTime = 0;
	vita49_payload_size = 1428;
}
/****************
 * Destructor
 ****************/
SinkVITA49_i::~SinkVITA49_i(void){
	BasicVRTPacket* temp2;
	destroy_tx_thread();
	boost::mutex::scoped_lock lock1( BankLock);
	while (! Bank2.empty()) {
		temp2 =  Bank2.front();
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
}
void SinkVITA49_i::memoryManagement(int maxPacketLength){
	destroy_tx_thread();
	if (spareBuffer != NULL)
		free(spareBuffer);
	spareBuffer = (char*)malloc(sizeof(char)*(maxPacketLength+20+8));
	createMem = false;

}
void SinkVITA49_i::stop() throw (CF::Resource::StopError, CORBA::SystemException){
	//LOG_DEBUG(SinkVITA49_i,__PRETTY_FUNCTION__);
	SinkVITA49_base::stop();
	close(uni_server.sock);
	destroy_tx_thread();
}

void SinkVITA49_i::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration){
	SinkVITA49_base::configure(props);
	for (CORBA::ULong ii = 0; ii < props.length(); ++ii) {
		boost::mutex::scoped_lock lock1(property_lock);
		LOG_DEBUG(SinkVITA49_i, "Configuring" << props[ii].id << "value = " << ossie::any_to_string(props[ii].value));
		const std::string id = (const char*) props[ii].id;

		if (id == "network_settings"){
			if(network_settings.enable){
				eth_dev = network_settings.interface;
				port = network_settings.port;
				ipAddress = CORBA::string_dup(network_settings.ip_address.c_str());
				vlan = network_settings.vlan;
				use_udp_protocol = network_settings.use_udp_protocol;

				if (port > 0 && !ipAddress.empty() && !eth_dev.empty()){
					if (!launch_tx_thread()){
						//detach(streamID);
						throw CF::PropertySet::InvalidConfiguration("SinkVITA49 could not connect to socket.",props);
					}
				}
			}
		}
		if (id == "VITA49Encapsulation"){
			VITAProcess.Encap.use_crc = VITA49Encapsulation.use_crc;
			VITAProcess.Encap.use_vrl_frames = VITA49Encapsulation.use_vrl_frames;
		}

		if (id == "VITA49IFDataPacket"){
			if (VITA49IFDataPacket.enable){
				VITAProcess.IFDPacket.enable = true;
				VITAProcess.IFDPacket.enable_class_identifier = VITA49IFDataPacket.enable_class_identifier;
				VITAProcess.IFDPacket.enable_stream_identifier = VITA49IFDataPacket.enable_stream_identifier;
				VITAProcess.IFDPacket.embed_time_stamp = VITA49IFDataPacket.embed_time_stamp;
				VITAProcess.IFDPacket.use_trailer = VITA49IFDataPacket.use_trailer;
			}
		}

		if (id == "VITA49IFContextPacket"){
			if (VITA49IFContextPacket.enable){
				VITAProcess.IFCPacket.enable = true;
				VITAProcess.IFCPacket.enable_class_identifier = VITA49IFContextPacket.enable_class_identifier;
				VITAProcess.IFCPacket.enable_device_identifier = VITA49IFContextPacket.enable_device_identifier;
				VITAProcess.IFCPacket.class_identifier = CORBA::string_dup(VITA49IFContextPacket.class_identifier.c_str());
				VITAProcess.IFCPacket.device_identifier = CORBA::string_dup(VITA49IFContextPacket.device_identifier.c_str());
				VITAProcess.IFCPacket.enable_stream_identifier = VITA49IFContextPacket.enable_stream_identifier;//
				VITAProcess.IFCPacket.embed_time_stamp = VITA49IFContextPacket.embed_time_stamp;
				VITAProcess.IFCPacket.use_trailer = VITA49IFContextPacket.use_trailer;
			}
		}
		if (id == "advanced_configuration"){
			int enumedType = advanced_configuration.endian_representation;

			if (enumedType == 1){
				if (BYTE_ORDER != __LITTLE_ENDIAN){
					_dataRef = __LITTLE_ENDIAN;
					convertEndian = true;
				}
			}
			else if (enumedType == 2){
				if (BYTE_ORDER != __BIG_ENDIAN){
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
			burstPacketCount = (int)advanced_configuration.number_of_packets_in_burst;
			_throttleTime = (int)advanced_configuration.throttle_time_between_packet_bursts;
		}
	}
}


void SinkVITA49_i::destroy_tx_thread(){
	if (_transmitThread != NULL){
		runThread = false;
		_transmitThread->join();
		delete _transmitThread;
		_transmitThread = NULL;
	}
	if (_contextThread != NULL){
		runThread = false;
		_contextThread->join();
		delete _contextThread;
		_contextThread = NULL;
	}
}

bool SinkVITA49_i::launch_tx_thread(){
	packetCount = 0;
	destroy_tx_thread();
	/* build the iterface string */
	std::ostringstream iface;
	iface << eth_dev;
	//connect to VLAN
	if (vlan != 0){
		iface << "." << vlan;
	}

	//check to see if this a multicast address or not
	if (inet_network(ipAddress.c_str()) > lowMulti && inet_network(ipAddress.c_str()) < highMulti && !ipAddress.empty())
	{
		LOG_DEBUG(SinkVITA49_i, "Enabling multicast_client on " << iface.str().c_str() << " " << ipAddress.c_str() << " " << port);
		server = multicast_server(iface.str().c_str(), ipAddress.c_str(), port);
		if (server.sock < 0) {
			LOG_ERROR(SinkVITA49_i, "Error: SinkVITA49_impl::RECEIVER() failed to connect to multicast socket");
			return false;
		}
		multicast = true;
	} else if (!ipAddress.empty()) {
		multicast = false;
		if (use_udp_protocol){

			LOG_DEBUG(SinkVITA49_i, "Enabling unicast_client on " << iface.str().c_str() << " " << ipAddress.c_str() << " " << port);
			uni_server = unicast_server(iface.str().c_str(), ipAddress.c_str(), port);
			if (uni_server.sock < 0) {
				std::cerr<< "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket  "<< std::endl;
				LOG_ERROR(SinkVITA49_i, "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket");
				return false;
			}
		}else{
			LOG_DEBUG(SinkVITA49_i, "TCP is not currently supported - defaulting to UDP " << iface.str().c_str() << " " << ipAddress.c_str() << " " << port);
			uni_server = unicast_server(iface.str().c_str(), ipAddress.c_str(), port);
			if (uni_server.sock < 0) {
				std::cerr<< "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket  "<< std::endl;
				LOG_ERROR(SinkVITA49_i, "Error: SinkVITA49::TRANSMITTER() failed to create unicast socket");
				return false;
			}

		}
	}
	runThread = true;

	if (multicast)
		_transmitThread = new boost::thread(&SinkVITA49_i::TRANSMITTER_M,this);
	else{
		_transmitThread = new boost::thread(&SinkVITA49_i::TRANSMITTER, this);
}
	if (timeOut > 0)
		_contextThread = new boost::thread(&SinkVITA49_i::timerThread,this);
	return true;
}

//create a context packet every X seconds
void SinkVITA49_i::timerThread(){
	while (runThread){
		sleep(timeOut);
		if (!waitingForSRI)
			createIFContextPacket(_tContext,0);
		boost::this_thread::interruption_point();
	}
}


void SinkVITA49_i::TRANSMITTER_M() {
	BasicVRTPacket *vrtPacket;
	BasicVRLFrame *vrl_frame= new BasicVRLFrame();
	int frameCounter = 0;
	long pCount = 0;
	while(runThread){
		boost::this_thread::interruption_point();
		if(!workQueue2.empty()){
			{
				if (_throttleTime > 0 && burstPacketCount == pCount){
					pCount = 0;
					usleep(_throttleTime);
				}
				boost::mutex::scoped_lock lock( workQueueLock);
				vrtPacket =  workQueue2.front();
				if (VITAProcess.Encap.use_vrl_frames){
					vrl_frame->setVRTPacket(vrtPacket);
					if (VITAProcess.Encap.use_crc)
						vrl_frame->updateCRC();
					vrl_frame->setFrameCount((++frameCounter) & 0xFFF);
					multicast_transmit(server,vrl_frame->getFramePointer(), vrl_frame->getFrameLength());
					pCount++;
				}else{
					multicast_transmit(server,vrtPacket->getPacketPointer(), vrtPacket->getPacketLength());
					pCount++;
				}
				workQueue2.pop();
			}
			delete vrtPacket;

		}else{
			usleep(1e5);
		}
		boost::this_thread::interruption_point();
	}
}
void SinkVITA49_i::TRANSMITTER() {
	BasicVRTPacket *vrtPacket;
	BasicVRLFrame *vrl_frame= new BasicVRLFrame();
	int frameCounter = 0;
	long pCount = 0;
	while(runThread){
		boost::this_thread::interruption_point();
		if(!workQueue2.empty()){
			{
				if (_throttleTime > 0 && burstPacketCount == pCount){
					pCount = 0;
					usleep(_throttleTime);
				}
				boost::mutex::scoped_lock lock( workQueueLock);
				vrtPacket =  workQueue2.front();
						if (VITAProcess.Encap.use_vrl_frames){
					vrl_frame->setVRTPacket(vrtPacket);
					if (VITAProcess.Encap.use_crc)
						vrl_frame->updateCRC();
					vrl_frame->setFrameCount((++frameCounter) & 0xFFF);
					unicast_transmit(uni_server,vrl_frame->getFramePointer(), vrl_frame->getFrameLength());
					pCount++;
				}else{
					unicast_transmit(uni_server,vrtPacket->getPacketPointer(), vrtPacket->getPacketLength());
					pCount++;
				}
				workQueue2.pop();
			}
			delete vrtPacket;

		}else{
			usleep(1e5);
		}
		boost::this_thread::interruption_point();
	}
}


int SinkVITA49_i::createPayload(int size, bool signed_v){
	int bytesPerPacket=0;
	int difference = 0;
	                                       //UDP HEADER                                  //VITA49 Header
	bytesPerPacket = vita49_payload_size + (8 + (VITAProcess.Encap.use_vrl_frames?12:0) + 4 + (VITAProcess.IFDPacket.enable_stream_identifier?4:0) + (VITAProcess.IFDPacket.enable_class_identifier?8:0) + (VITAProcess.IFDPacket.embed_time_stamp?12:0) + (VITAProcess.IFDPacket.use_trailer?4:0));
	LOG_DEBUG(SinkVITA49_i, " bytes per if data stream packet are " << bytesPerPacket);
	//MAX IPV4 UDP is 65535
	//IPV4 header is 20 bytes
	if (bytesPerPacket > 65515){
		//find the difference
		difference = bytesPerPacket-65515;
		bytesPerPacket=bytesPerPacket-difference;
	}

	connection_status.packet_size = bytesPerPacket;

	RealComplexType type;
	if(currSRI.mode == 0)
		type=RealComplexType_Real;
	else
		type=RealComplexType_ComplexCartesian;

	pf->setRealComplexType(type);

	DataType dType;
	DataItemFormat format;

	int fieldSize = 0;

	if (signed_v){
		if (size == 1)
			dType = DataType_Int8;
		else
			dType = DataType_Int16;
		format = DataItemFormat_SignedInt;
	}else{
		if (size == 1)
			dType = DataType_UInt8;
		else
			dType = DataType_UInt16;
		format = DataItemFormat_UnsignedInt;
	}
	fieldSize = size*8;

	if (size == 4){
		format = DataItemFormat_Float;
		dType = DataType_Float;
	}
	else if (size == 8){
		format = DataItemFormat_Double;
		dType = DataType_Double;
	}
	pf->setDataItemSize(fieldSize);
	pf->setDataType(dType);
	pf->setDataItemFormat(format);
	pf->setItemPackingFieldSize(fieldSize);

	if (difference > 0){
		int subtract_bytes = (difference - ((1*currSRI.mode+1)*size)) - 1 - (difference-1)%((1*currSRI.mode+1)*size);
		vita49_payload_size -= subtract_bytes;
	}
	samplesPerPacket = floor(vita49_payload_size/((1*currSRI.mode+1)*size));

	//add set classid here
	//create the classid assuming we are using a standardPacket
	StandardDataPacket *newP = new StandardDataPacket();
	newP->setPayloadFormat(pf->getBits());
	standardPacketClassID = newP->getClassID();
	delete newP;

	return 1;
}
void SinkVITA49_i::createPacket(BasicDataPacket* pkt, TimeStamp vrt_ts, int sampleIndex_l){
	/* use the vector magic to make this work */
	std::_Vector_base<char, _seqVector::seqVectorAllocator<char> >::_Vector_impl *vectorPointer = (std::_Vector_base<char, _seqVector::seqVectorAllocator<char> >::_Vector_impl *) ((void*) & (standardDPacket->bbuf));
	vectorPointer->_M_start = const_cast<char*>(&pkt->bbuf[0]);
	vectorPointer->_M_finish = vectorPointer->_M_start + pkt->bbuf.size();
	vectorPointer->_M_end_of_storage = vectorPointer->_M_finish;

	std::string streamID = CORBA::string_dup(currSRI.streamID);

	if (VITAProcess.IFDPacket.enable_stream_identifier){
		//standardDPacket->setStreamIdentifier(_streamMap[streamID]+streamIDoffset);
		standardDPacket->setStreamIdentifier(_streamMap.hash+streamIDoffset);

	}
	if (VITAProcess.IFDPacket.enable_class_identifier){
		//sets the class_identifier and the payload format
		standardDPacket->setPayloadFormat(pf->getBits());
	}


	standardDPacket->setTimeStamp(vrt_ts);
	standardDPacket->setPacketCount(packetCount%16);

	if (VITAProcess.IFDPacket.use_trailer){
		standardDPacket->setAssocPacketCount(contextCount%127);
	}
    //match the class_identifierentifier of the context packet
	packetCount++;
}

int SinkVITA49_i::createIFContextPacket(BULKIO::PrecisionUTCTime t, int index){
	BasicContextPacket* pkt = new BasicContextPacket();
	TimeStamp ts;
	ts = calcNextTimeStamp(t,(double)currSRI.xdelta,index);
	//createIFContextPacket(cp,nextTimeStamp);
	bool changed = false;
	/* fill out the packet will all fields per the VITA49 Spec */
	pkt->setChangePacket(false);
	pkt->setReferencePointIdentifier(0);
	pkt->setBandwidth(0.0);
	pkt->setFrequencyIF(0.0);
	pkt->setFrequencyRF(0.0);
	pkt->setFrequencyOffsetRF(0.0);
	pkt->setBandOffsetIF(0.0);
	pkt->setReferenceLevel(0.0);
	pkt->setGain(0.0);
	//pkt->setGain1(0.0); //?
	//pkt->setGain2(0.0); //?
	pkt->setOverRangeCount(0);
	pkt->setSampleRate(0.0);
	//pkt->setSamplePeriod(0.0); //?
	pkt->setTimeStampAdjustment(0);
	pkt->setTimeStampCalibration(0);
	pkt->setTemperature(0);
	pkt->setCalibratedTimeStamp(_FALSE);
	pkt->setDataValid(_FALSE);
	pkt->setReferenceLocked(_FALSE);
	pkt->setAutomaticGainControl(_FALSE);
	pkt->setSignalDetected(_FALSE);
	pkt->setInvertedSpectrum(_FALSE); //?
	pkt->setOverRange(_FALSE);
	pkt->setDiscontinuous(_FALSE);
	pkt->setDataPayloadFormat(pf->getBits());
	pkt->setPacketCount((++contextCount)&& 0xF);
	pkt->setUserDefinedBits(0); //?

	std::string streamID = CORBA::string_dup(currSRI.streamID);

	//if ( _streamMap.find(streamID) == _streamMap.end() ) {
	if (strcmp(_streamMap.streamID.c_str(),currSRI.streamID) != 0){
		LOG_ERROR(SinkVITA49_i, currSRI.streamID << " Does not Match " << _streamMap.streamID);
		//the stream id was not found in the stream map
		return NOOP;
	}
	if (VITAProcess.IFCPacket.enable_stream_identifier)
		pkt->setStreamIdentifier(_streamMap.hash);// The stream ID

		//pkt->setStreamIdentifier(_streamMap[streamID]);// The stream ID
	if(VITAProcess.IFCPacket.enable_class_identifier){
	if (strcmp(VITAProcess.IFCPacket.class_identifier.c_str(), "DEFAULT")==0){
		pkt->setClassID(standardPacketClassID);
		VITAProcess.IFCPacket.class_identifier = standardPacketClassID;
	}else{
		std::string classID (VITAProcess.IFCPacket.class_identifier);
		pkt->setClassID(classID);
	}
	}
	if (VITAProcess.IFCPacket.enable_device_identifier){
	std::string device_identifier (VITAProcess.IFCPacket.device_identifier);
	pkt->setDeviceID(device_identifier);
	if (VITAProcess.IFCPacket.embed_time_stamp){
		pkt->setTimeStamp(ts);
	}
	}
	pkt->setDataPayloadFormat(pf->getBits());
	//pkt->setChangePacket(true);
	pkt->setSampleRate(1.0/currSRI.xdelta);
	//pkt->setSamplePeriod(currSRI.xdelta);

	double value_d;
	float value_f;
	//long long value_ll;
	long value_l;
	bool value_b;

	unsigned long currSize = currSRI.keywords.length();
	for (unsigned long i = 0;i<currSize;++i){
		if (strcmp("COL_BW",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;

			pkt->setBandwidth(value_d);
			changed = true;
		}
		else if (strcmp("COL_IF_FREQUENCY",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setFrequencyIF(value_d);
			changed = true;
		}
		else if (strcmp("COL_RF",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setFrequencyRF(value_d);
			changed = true;
		}
		else if (strcmp("COL_RF_OFFSET",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setFrequencyOffsetRF(value_d);
			changed = true;
		}
		else if (strcmp("COL_IF_FREQUENCY_OFFSET",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setBandOffsetIF(value_d);
			changed = true;
		}
		else if (strcmp("COL_REFERENCE_LEVEL",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_f;
			pkt->setReferenceLevel(value_f);
			changed = true;
		}
		else if (strcmp("REFERENCE_POINT_IDENTIFIER",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_l;
			pkt->setReferencePointIdentifier(value_l);
			changed = true;
		}
		else if (strcmp("COL_GAIN",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_f;
			pkt->setGain1(value_f);
			changed = true;
		}
		else if (strcmp("DATA_GAIN",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_f;
			pkt->setGain2(value_f);
			changed = true;
		}
		else if (strcmp("OVER_RANGE_SUM",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setOverRangeCount((long long)value_d);
			changed = true;
		}
		else if (strcmp("USER_DEFINED",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_l;
			pkt->setUserDefinedBits((int32_t)value_l);
			changed = true;
		}
		else if (strcmp("TIMESTAMP_ADJUSTMENT_PICOSECONDS",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_d;
			pkt->setTimeStampAdjustment((long long)value_d);
			changed = true;
		}
		else if (strcmp("TIMESTAMP_CALIBRATION",currSRI.keywords[i].id) == 0){
			currSRI.keywords[i].value >>= value_l;
			pkt->setTimeStampCalibration((int32_t)value_l);
			changed = true;
		}
		else if (strcmp("TEMPERATURE",currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_f;
			pkt->setTemperature(value_f);
			changed = true;
		}
		else if (strcmp("DATA_VALID",currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setDataValid(_TRUE);
			if (!value_b)
				pkt->setDataValid(_FALSE);
			changed = true;
		}
		else if (strcmp("REFERENCE_LOCKED",currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setReferenceLocked(_TRUE);
			if (!value_b)
				pkt->setReferenceLocked(_FALSE);
			changed = true;
		}
		else if (strcmp("CALIBRATED_TIME_STAMP", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setCalibratedTimeStamp(_TRUE);
			if (!value_b)
				pkt->setCalibratedTimeStamp(_FALSE);
			changed = true;
		}
		else if (strcmp("AUTO_GAIN_CONTROL", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setAutomaticGainControl(_TRUE);
			if (!value_b)
				pkt->setAutomaticGainControl(_FALSE);
			changed = true;
		}
		else if (strcmp("SIGNAL_DETECTION", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setSignalDetected(_TRUE);
			if (!value_b)
				pkt->setSignalDetected(_FALSE);
			changed = true;
		}
		else if (strcmp("DATA_INVERSION", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setInvertedSpectrum(_TRUE);
			if (!value_b)
				pkt->setInvertedSpectrum(_FALSE);
			changed = true;
		}
		else if (strcmp("OVER_RANGE", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setOverRange(_TRUE);
			if (!value_b)
				pkt->setOverRange(_FALSE);
			changed = true;
		}
		else if (strcmp("SAMPLE_LOSS", currSRI.keywords[i].id)==0){
			currSRI.keywords[i].value >>= value_b;
			if (value_b)
				pkt->setDiscontinuous(_TRUE);
			if (!value_b)
				pkt->setDiscontinuous(_FALSE);
			changed = true;
		}

		else if (strcmp("GEOLOCATION_GPS",currSRI.keywords[i].id)==0){
			GEOLOCATION_GPS_struct geolocation_gps;
			currSRI.keywords[i].value >>= geolocation_gps;
			TimeStamp *time = new TimeStamp(IntegerMode_GPS,geolocation_gps.TIME_SECONDS,geolocation_gps.TIME_FRACTIONAL*1e9,1.0/currSRI.xdelta);
			processingGeolocation.setTimeStamp(*time);
			processingGeolocation.setAltitude(geolocation_gps.ALTITUDE);
			processingGeolocation.setHeadingAngle(geolocation_gps.HEADING_ANGLE);
			processingGeolocation.setLatitude(geolocation_gps.LATITUDE);
			processingGeolocation.setLongitude(geolocation_gps.LONGITUDE);
			processingGeolocation.setMagneticVariation(geolocation_gps.MAGNETIC_VARIATION);
			processingGeolocation.setManufacturerIdentifier(geolocation_gps.MANUFACTURER_ID);
			processingGeolocation.setSpeedOverGround(geolocation_gps.GROUND_SPEED);
			processingGeolocation.setTrackAngle(geolocation_gps.TRACK_ANGLE);

			pkt->setGeolocationGPS(processingGeolocation);
			delete time;
		}

		else if (strcmp("GEOLOCATION_INS",currSRI.keywords[i].id)==0){
			GEOLOCATION_INS_struct geolocation_ins;
			currSRI.keywords[i].value >>= geolocation_ins;
			TimeStamp *time = new TimeStamp(IntegerMode_GPS,geolocation_ins.TIME_SECONDS,geolocation_ins.TIME_FRACTIONAL*1e9,1.0/currSRI.xdelta);
			processingGEOINS.setTimeStamp(*time);
			processingGEOINS.setAltitude(geolocation_ins.ALTITUDE);
			processingGEOINS.setHeadingAngle(geolocation_ins.HEADING_ANGLE);
			processingGEOINS.setLatitude(geolocation_ins.LATITUDE);
			processingGEOINS.setLongitude(geolocation_ins.LONGITUDE);
			processingGEOINS.setMagneticVariation(geolocation_ins.MAGNETIC_VARIATION);
			processingGEOINS.setManufacturerIdentifier(geolocation_ins.MANUFACTURER_ID);
			processingGEOINS.setSpeedOverGround(geolocation_ins.GROUND_SPEED);
			processingGEOINS.setTrackAngle(geolocation_ins.TRACK_ANGLE);

			pkt->setGeolocationINS(processingGEOINS);
			delete time;
		}

		else if (strcmp("EPHEMERIS_ECEF",currSRI.keywords[i].id)==0){
			EPHEMERIS_ECEF_struct ephemeris_ecef;
			currSRI.keywords[i].value >>= ephemeris_ecef;
			TimeStamp *time = new TimeStamp(IntegerMode_GPS,ephemeris_ecef.TIME_SECONDS,ephemeris_ecef.TIME_FRACTIONAL_SECONDS*1e9,1.0/currSRI.xdelta);
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
			/* KNOWN BUG - ADJUNCT EPHEMERIS CLASS DOES NOT WORK AS EXPECTED
			//EphemerisAdjunct Adjunct;
			//processingEphemeris.setAdjunct(Adjunct);
			*/
			processingEphemeris.setRotationalVelocityAlpha(ephemeris_ecef.ROTATIONAL_VELOCITY_ALPHA);
			processingEphemeris.setRotationalVelocityBeta(ephemeris_ecef.ROTATIONAL_VELOCITY_BETA);
			processingEphemeris.setRotationalVelocityPhi(ephemeris_ecef.ROTATIONAL_VELOCITY_PHI);
			processingEphemeris.setAccelerationX(ephemeris_ecef.ACCELERATION_X);
			processingEphemeris.setAccelerationY(ephemeris_ecef.ACCELERATION_Y);
			processingEphemeris.setAccelerationZ(ephemeris_ecef.ACCELERATION_Z);
			processingEphemeris.setRotationalAccelerationAlpha(ephemeris_ecef.ROTATIONAL_ACCELERATION_ALPHA);
			processingEphemeris.setRotationalAccelerationBeta(ephemeris_ecef.ROTATIONAL_ACCELERATION_BETA);
			processingEphemeris.setRotationalAccelerationPhi(ephemeris_ecef.ROTATIONAL_ACCELERATION_PHI);

			pkt->setEphemerisECEF(processingEphemeris);
		}

		else if (strcmp("EPHEMERIS_RELATIVE",currSRI.keywords[i].id)==0){
			EPHEMERIS_RELATIVE_struct ephemeris_relative;
			currSRI.keywords[i].value >>= ephemeris_relative;
			TimeStamp *time = new TimeStamp(IntegerMode_GPS,ephemeris_relative.TIME_SECONDS,ephemeris_relative.TIME_FRACTIONAL_SECONDS*1e9,1.0/currSRI.xdelta);
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
			// KNOWN BUG - ADJUNCT EPHEMERIS CLASS DOES NOT WORK AS EXPECTED
			processingEphemerisRel.setRotationalVelocityAlpha(ephemeris_relative.ROTATIONAL_VELOCITY_ALPHA);
			processingEphemerisRel.setRotationalVelocityBeta(ephemeris_relative.ROTATIONAL_VELOCITY_BETA);
			processingEphemerisRel.setRotationalVelocityPhi(ephemeris_relative.ROTATIONAL_VELOCITY_PHI);
			processingEphemerisRel.setAccelerationX(ephemeris_relative.ACCELERATION_X);
			processingEphemerisRel.setAccelerationY(ephemeris_relative.ACCELERATION_Y);
			processingEphemerisRel.setAccelerationZ(ephemeris_relative.ACCELERATION_Z);
			processingEphemerisRel.setRotationalAccelerationAlpha(ephemeris_relative.ROTATIONAL_ACCELERATION_ALPHA);
			processingEphemerisRel.setRotationalAccelerationBeta(ephemeris_relative.ROTATIONAL_ACCELERATION_BETA);
			processingEphemerisRel.setRotationalAccelerationPhi(ephemeris_relative.ROTATIONAL_ACCELERATION_PHI);
			delete time;
			//pkt->setEphemerisRelative(processingEphemerisRel);
		}
	}
	pkt->setChangePacket(changed);
	{
		boost::mutex::scoped_lock lock(workQueueLock);
		workQueue2.push(pkt);
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
 *   returns NORMAL if one or more of those ports received and processed data. It also
 *   handles keeping track of the SoS bit and reflips it if a break in transmission occurs,
 *   i.e. no data being received for the timeoutInSec_IN_SEC (set to 2)
 *********************************************************************************************/
int SinkVITA49_i::serviceFunction(){
	bool retService;
	if (createMem){
		memoryManagement(vita49_payload_size);
	}
	if (_transmitThread == NULL){
		launch_tx_thread();
	}
	//assumes only one provides port is active at a time
	retService = singleService(dataDouble_in,false);
	retService = retService || singleService(dataFloat_in, false);
	retService = retService || singleService(dataUshort_in, false);
	retService = retService || singleService(dataShort_in, true);
	retService = retService || singleService(dataChar_in,true);
	retService = retService || singleService(dataOctet_in, false);

	if (retService){
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
 *   number of packets out of the SDDS bank one at time, populates the packet, and pushes it into
 *   the work queue.  It then signals the transmitter that there is a new packet ready to
 *   transmit.  Some special notes... If the size of the data returned by the getPacket call is
 *   not a multiple of 1024, the remainder bytes are stored in a packet at the front of the
 *   SDDS bank and that packets header is set but the packet is left in the SDDS bank (unless
 *   the EOS flag was received in which case the rest of the data will be zero filled and the
 *   packet will be pulled from the SDDS bank and pushed into the work queue.  The next piece
 *   of data that comes in will be used to fill the rest of the packet at which point it will
 *   be pulled from the SDDS Bank and pushed into the work queue.
 *************************************************************************************************/
template <class IN> bool SinkVITA49_i::singleService(IN *dataIn, bool signedPort){
	LOG_DEBUG(SinkVITA49_i, __PRETTY_FUNCTION__);
	typename IN::dataTransfer *CORBApacket = dataIn->getPacket(0);
	if (CORBApacket == NULL)
		return false;
	BasicDataPacket *vrtPacket = NULL;
	//	BasicContextPacket *cp = NULL;
	int sampleSize = sizeof(CORBApacket->dataBuffer.front());

	if (_streamMap.hash == 0){
		//we need a hash of the string to create the stream ID for the context packet
		boost::hash<std::string> string_hash;
		//streamID = CORBApacket->streamID;
		string_hash(CORBApacket->streamID);
		//_streamMap.insert(std::make_pair(CORBApacket->streamID,(unsigned int)string_hash(CORBApacket->streamID)));
		//_streamMap[CORBApacket->streamID] = (unsigned int)string_hash(CORBApacket->streamID);
		_streamMap.hash = (unsigned int)string_hash(CORBApacket->streamID);
		_streamMap.streamID = CORBApacket->streamID;
		initstreamDef(sampleSize, signedPort);

		addModifyKeyword<long>(&currSRI,"dataRef",_dataRef, true);
		addModifyKeyword<bool>(&currSRI,"BULKIO_SRI_PRIORITY",_bulkioPriority, true);

		((BULKIO_dataVITA49_Out_implemented*)dataVITA49_out)->attach(_streamDef, CORBApacket->streamID.c_str());
	}else{
		if ( strcmp(_streamMap.streamID.c_str(),CORBApacket->streamID.c_str()) != 0){
			//log an error
			LOG_ERROR(SinkVITA49_i, "SinkVITA49 can not handle multiple streams on input, streamID:" << CORBApacket->SRI.streamID << " " );
			return NOOP;


		}
	}

	if (CORBApacket->sriChanged || waitingForSRI){
		bool t = mergeRecSRI(CORBApacket->SRI, CORBApacket->T);
		if (t && VITAProcess.IFCPacket.enable){
			createPayload(sampleSize,signedPort);
			//cp = new BasicContextPacket();
			//nextTimeStamp = calcNextTimeStamp(CORBApacket->T,currSRI.xdelta,dataIndex);
			if (leftOverDataSize == 0)
				createIFContextPacket(CORBApacket->T,0);
			else
				createIFContextPacket(CORBApacket->T,-(leftOverDataSize/(sampleSize))/(1*currSRI.mode+1));
		}
		waitingForSRI = false;
	}
	//process the data
	int numSamples = 0;
	if ( leftOverDataSize!= 0)
		numSamples = (CORBApacket->dataBuffer.size())/(1+currSRI.mode) + (leftOverDataSize/sampleSize)/(1*currSRI.mode+1);
	else
		numSamples = (CORBApacket->dataBuffer.size())/(1+currSRI.mode);
	int dataSizeInBytes = numSamples*sampleSize;

	while(dataSizeInBytes >= (samplesPerPacket*((1*currSRI.mode+1)*sampleSize))){
		vrtPacket = new BasicDataPacket();
		vrtPacket->setPayloadFormat(pf->getBits());
		//PayloadFormat pf_l = vrtPacket->getPayloadFormat();
		vrtPacket->setPayloadLength(samplesPerPacket*((1*currSRI.mode+1)*sampleSize));
		nextTimeStamp = calcNextTimeStamp(CORBApacket->T, (double)currSRI.xdelta,dataIndex);
		createPacket(vrtPacket,nextTimeStamp,dataIndex);
		if (leftOverDataSize > 0){
			//memcpy(spareBuffer,leftoverCORBApacket->dataBuffer.data()+(leftoverCORBApacket->dataBuffer.size()-leftOverDataSize),leftOverDataSize*sampleSize);
			memcpy(&spareBuffer[leftOverDataSize],CORBApacket->dataBuffer.data(),(samplesPerPacket*((1*currSRI.mode+1)*sampleSize))-leftOverDataSize);
			nextTimeStamp = calcNextTimeStamp(CORBApacket->T,(double)currSRI.xdelta,-(leftOverDataSize/sampleSize)/(1*currSRI.mode+1));
			vrtPacket->setTimeStamp(nextTimeStamp);
			vrtPacket->setData(pf->getBits(),spareBuffer,samplesPerPacket*((1*currSRI.mode+1)*sampleSize),convertEndian);
			dataIndex += (samplesPerPacket*(1*currSRI.mode+1))-(leftOverDataSize/sampleSize)/(1*currSRI.mode+1);
			leftOverDataSize = 0;
		}else{
			//nextTimeStamp = calcNextTimeStamp(CORBApacket->T,currSRI.xdelta,dataIndex);
			vrtPacket->setData(pf->getBits(),&CORBApacket->dataBuffer[dataIndex],samplesPerPacket*((1*currSRI.mode+1)*sampleSize),convertEndian);
			dataIndex += (samplesPerPacket*(1*currSRI.mode+1));
		}
		dataSizeInBytes -= (samplesPerPacket*((1*currSRI.mode+1)*sampleSize));
		//update the context packet time
		_tContext.tfsec = nextTimeStamp.getFractionalSeconds()/10e9;
		_tContext.twsec = nextTimeStamp.getUTCSeconds();
		LOG_DEBUG(SinkVITA49_i, __PRETTY_FUNCTION__ << " ABOUT TO PUSH TO QUEUE ");
		{
			boost::mutex::scoped_lock lock(workQueueLock);
			workQueue2.push(vrtPacket);
		}
	}

	if ( dataSizeInBytes > 0){
		memcpy(&spareBuffer[0], &CORBApacket->dataBuffer[dataIndex],dataSizeInBytes);
		leftOverDataSize = dataSizeInBytes; //in bytes
	}

	if (CORBApacket->EOS){
		if (dataSizeInBytes > 0){
			vrtPacket = new BasicDataPacket();
			vrtPacket->setPayloadFormat(pf->getBits());
			vrtPacket->setPayloadLength(dataSizeInBytes*((1*currSRI.mode+1)*sampleSize));
			nextTimeStamp = calcNextTimeStamp(CORBApacket->T, (double)currSRI.xdelta,dataIndex);
			createPacket(vrtPacket,nextTimeStamp,dataIndex);

			vrtPacket->setData(pf->getBits(),&spareBuffer[0],dataSizeInBytes*((1*currSRI.mode+1)*sampleSize),convertEndian);
			{
				boost::mutex::scoped_lock lock(workQueueLock);
				workQueue2.push(vrtPacket);
			}
		}
		_streamMap.hash = 0;
		_streamMap.streamID = "";
		((BULKIO_dataVITA49_Out_implemented*)dataVITA49_out)->detach();
	}

	dataIndex = 0;

	/* delete the dataTransfer object */
	delete CORBApacket;

	return true;
}

void SinkVITA49_i::initstreamDef(int sampleSize, bool signedPort){
	//always required info
	_streamDef.vlan = vlan;
	_streamDef.port = port;
	_streamDef.ip_address = CORBA::string_dup(ipAddress.c_str());
	if (use_udp_protocol)
		_streamDef.protocol = BULKIO::VITA49_UDP_TRANSPORT;
	else
		_streamDef.protocol = BULKIO::VITA49_TCP_TRANSPORT;

	//if we are not sending context packets setup the attach information

	if (VITA49IFContextPacket.enable){
		_streamDef.valid_data_format = true;
		_streamDef.data_format.packing_method_processing_efficient = true;

		if (currSRI.mode == 1)
			_streamDef.data_format.complexity = BULKIO::VITA49_COMPLEX_CARTESIAN;
		else
			_streamDef.data_format.complexity = BULKIO::VITA49_REAL;

		if ( sampleSize == 4){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_32F;
			_streamDef.data_format.item_packing_field_size = 32;
			_streamDef.data_format.data_item_size    = 32;
		}if (sampleSize == 8){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_64F;
			_streamDef.data_format.item_packing_field_size = 64;
			_streamDef.data_format.data_item_size    = 64;
		}
		if (signedPort && sampleSize ==2){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_16T;
			_streamDef.data_format.item_packing_field_size = 16;
			_streamDef.data_format.data_item_size    = 16;

		}else if (!signedPort && sampleSize == 2){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_16U;
			_streamDef.data_format.item_packing_field_size = 16;
			_streamDef.data_format.data_item_size    = 16;
		}else if (signedPort && sampleSize == 1){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_8T;
			_streamDef.data_format.item_packing_field_size = 8;
			_streamDef.data_format.data_item_size    = 8;
		}else if (!signedPort && sampleSize == 1){
			_streamDef.data_format.data_item_format = BULKIO::VITA49_8T;
			_streamDef.data_format.item_packing_field_size = 8;
			_streamDef.data_format.data_item_size    = 8;
		}
		_streamDef.data_format.repeat_count = 2;
		_streamDef.data_format.vector_size = 0;

	}else
		_streamDef.valid_data_format = false;
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
}

bool SinkVITA49_i::compareSRI(BULKIO::StreamSRI A, BULKIO::StreamSRI B){
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

	for(unsigned int i = 0; i < A.keywords.length(); i++) {
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
	if (!compareSRI(recSRI,currSRI)) {
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

		for (unsigned long i = 0;i<keySize;++i){
			//want to search all of currSize
			std::string action = "eq";
			//look for the id and set to true of found
			bool foundID = false;
			for (unsigned long j = 0;j<currSize;++j){
				if (strcmp(recSRI.keywords[i].id,currSRI.keywords[j].id) == 0) {
					foundID = true;
					if (!ossie::compare_anys(recSRI.keywords[i].value, currSRI.keywords[j].value, action)) {
						updateSRI = true;
						currSRI.keywords[j].value = recSRI.keywords[i].value;
					}
				}
			}
			//the id was not found we need to extend the length of keywords in currSRI and add this one to the end
			if (!foundID){
				updateSRI = true;
				unsigned long keySize_t = currSRI.keywords.length();
				currSRI.keywords.length(keySize_t + 1);
				currSRI.keywords[keySize_t].id = CORBA::string_dup(recSRI.keywords[i].id);
				currSRI.keywords[keySize_t].value = recSRI.keywords[i].value;
			}
		}

		if(updateSRI){
			//printSRI(&currSRI," Merged SRI");
			((BULKIO_dataVITA49_Out_implemented*)dataVITA49_out)->pushSRI(currSRI,time);
		}
	}
	return updateSRI;
}
