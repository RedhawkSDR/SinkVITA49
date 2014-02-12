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

#ifndef SINKVITA49_H
#define SINKVITA49_H

#include "SinkVITA49_base.h"
#include "VRTObject.h"
#include "BasicVRLFrame.h"
#include "BasicVRTPacket.h"
#include "StandardDataPacket.h"
#include "BasicDataPacket.h"
#include "BasicContextPacket.h"
#include <boost/functional/hash.hpp>
#include "BULKIO_dataVITA49_Out_implemented.h"
#include "multicast.h"
#include "unicast.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include "VITA49_struct_keywords.h"

struct VITA49Settings{
	VITA49Encapsulation_struct Encap;
	VITA49IFDataPacket_struct IFDPacket;
	VITA49IFContextPacket_struct IFCPacket;
} ;
class SinkVITA49_i;

class SinkVITA49_i : public SinkVITA49_base
{
	ENABLE_LOGGING
public:
	SinkVITA49_i(const char *uuid, const char *label);
	void __constructor__();
	~SinkVITA49_i();
	void configure(const CF::Properties&) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration);
	int serviceFunction();
	template <class IN> bool singleService(IN *dataIn, bool value);
	void TRANSMITTER();
	void TRANSMITTER_M();
	bool compareSRI(BULKIO::StreamSRI A, BULKIO::StreamSRI B);
	bool mergeRecSRI(BULKIO::StreamSRI &recSRI, BULKIO::PrecisionUTCTime T);
	void setDefaultSRI();
	int createPayload(int, bool);
	void createPacket(vrt::BasicDataPacket* pkt, TimeStamp T, int index);
	//int createIFContextPacket(vrt::BasicContextPacket* pkt, vrt::TimeStamp ts);
	int createIFContextPacket(BULKIO::PrecisionUTCTime t, int index);
	void stop() throw (CF::Resource::StopError, CORBA::SystemException);
private:
	//BULKIO_dataVITA49_Out_implemented *dataVITA49_out;
	void initialize_values();
	void memoryManagement(int maxPacketLength);
	void createIFContextHeader();
	void initstreamDef(int sampleSize, bool signedPort);
	void timerThread();
	std::string standardPacketClassID;
	std::string streamID;
	TimeStamp nextTimeStamp;
	BULKIO::VITA49StreamDefinition _streamDef;
	boost::thread* _transmitThread;
	boost::thread* _contextThread;
	//boost::thread* _timerThread;
	boost::asio::io_service io;
	boost::asio::deadline_timer *t;
	bool timer_valid;
	boost::mutex running_lock;
	boost::mutex property_lock;
	boost::mutex sriLock;
	bool launch_tx_thread();
	void destroy_tx_thread();
	bool runThread;
	bool waitForContext;
	bool tx_thead_running;
	bool remainingData;
	uint8_t* remainingSamples;
	int remainingSize;
	multicast_t server;
	unicast_t uni_server;
	long timeOut;
	PayloadFormat *pf;
	BasicContextPacket *contextPacket;

	struct{
		std::string streamID;
		unsigned int hash;
	}_streamMap;

	//std::map<std::string, unsigned int> _streamMap;
	std::map<std::string, std::string>  _attachMap;

	int vlan;
	std::string ipAddress;
	std::string eth_dev;
	int port;
	bool use_udp_protocol;
	bool multicast;
	unsigned long lowMulti;
	unsigned long highMulti;

	VITA49Settings VITAProcess;
	bool waitingForSRI;

	omni_mutex dataAvailableMutex;
	omni_condition* dataAvailableSignal;

	boost::mutex BankLock;
	std::queue<BasicVRTPacket* > Bank2;

	boost::mutex workQueueLock;
	std::queue<BasicVRTPacket* > workQueue2;

	bool createMem;
	long numBuffers;

	bool _bulkioPriority;
	long _dataRef;

	bool convertEndian;

	BULKIO::StreamSRI currSRI;
	void printSRI(BULKIO::StreamSRI *sri, std::string strHeader = "DEBUG SRI"){
		std::cout << strHeader << ":\n";
		std::cout << "\thversion: " << sri->hversion<< std::endl;
		std::cout << "\txstart: " << sri->xstart<< std::endl;
		std::cout << "\txdelta: " << sri->xdelta<< std::endl;
		std::cout << "\txunits: " << sri->xunits<< std::endl;
		std::cout << "\tsubsize: " <<sri->subsize<< std::endl;
		std::cout << "\tystart: " << sri->ystart<< std::endl;
		std::cout << "\tydelta: " << sri->ydelta<< std::endl;
		std::cout << "\tyunits: " << sri->yunits<< std::endl;
		std::cout << "\tmode: " << sri->mode<< std::endl;
		std::cout << "\tstreamID: " << sri->streamID<< std::endl;
		for (size_t i = 0; i < sri->keywords.length(); i++) {
			std::cout << "\t KEYWORD KEY/VAL :: " << sri->keywords[i].id << ": " << ossie::any_to_string(sri->keywords[i].value) << std::endl;
		}
		std::cout << std::endl;
	}
	template <typename CORBAXX>
		bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
			CORBA::Any value;
			value <<= myValue;
			unsigned long keySize = sri->keywords.length();
			if (!addOnly) {
				for (unsigned int i = 0; i < keySize; i++) {
					if (!strcmp(sri->keywords[i].id, id)) {
						sri->keywords[i].value = value;
						return true;
					}
				}
			}
			sri->keywords.length(keySize + 1);
			if (sri->keywords.length() != keySize + 1)
				return false;
			sri->keywords[keySize].id = CORBA::string_dup(id);
			sri->keywords[keySize].value = value;
			return true;
		}
	int dataIndex;
	int packetCount;
	int contextCount;
	int samplesPerPacket;
	TimeStamp calcNextTimeStamp (BULKIO::PrecisionUTCTime T_v, double xdelta, int dataIndex_v){
		BULKIO::PrecisionUTCTime ts = T_v;
		TimeStamp curr;

		double time = dataIndex_v*xdelta;
		//std::cout << dataIndex << std::endl;
		//std::cout << time << std::endl;
		//std::cout << ts.twsec << std::endl;
		//std::cout << ts.tfsec << std::endl;
		if (dataIndex < 0){
			if (abs(dataIndex) > abs(ts.tfsec)){
				ts.twsec -= 1.0;
				ts.tfsec = (1.0+ts.tfsec) + time;
			}else{
				ts.tfsec -= time;
			}
		}else{
			ts.tfsec += time;
			if (ts.tfsec >= 1.0){
				ts.twsec += floor(ts.tfsec);
				ts.tfsec = ts.tfsec-floor(ts.tfsec);
			}
		}
		curr = TimeStamp(IntegerMode_UTC, ts.twsec, ts.tfsec*1.0e12);
		return curr;
	}

	BULKIO::PrecisionUTCTime _tContext;


	char* spareBuffer;
	int _throttleTime;

	StandardDataPacket *standardDPacket;
	unsigned int leftOverDataSize;
	bool was_it_valid;

	Ephemeris processingEphemeris;
	Ephemeris processingEphemerisRel;
	Geolocation processingGeolocation;
	Geolocation processingGEOINS;
	GeoSentences processingGeoSentences;
	GEOLOCATION_GPS_struct geolocation_structure;
	int streamIDoffset;
	int vita49_payload_size;
	long burstPacketCount;
};


#endif

