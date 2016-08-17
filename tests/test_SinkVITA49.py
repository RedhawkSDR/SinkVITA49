#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

DEBUG_LEVEL = 3

import unittest
import ossie.utils.testing
import os
from omniORB import any, CORBA
from ossie.cf import CF
from ossie.utils.bulkio import bulkio_data_helpers
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
import bulkio
from ossie.utils import sb
import time, socket, struct


class vita49_data_header(object):
    
    def __init__(self,header):
        self.vrlframe,byte0,byte1,self.packet_size,self.streamID,self.reserved_word,byte3,byte4,self.vector_size,self.wsec,self.fsec = struct.unpack("!I2BHIIBBHIQ", header)
        self.packet_type = byte0 >>4
        # Data Packet
        if self.packet_type == 1: 
            self.has_class_ID = (byte0 >>3 ) & 0x01
            self.has_trailer = (byte0 >>2 ) & 0x01
            self.tsi = (byte1 >>6 ) & 0x03
            self.tsf = (byte1 >>4 ) & 0x03
            self.packet_count = byte1  & 0x0F
            self.complex = (byte4 >>4) & 0x01
            self.data_type = byte4 & 0x0F
        # Context Packet
        if self.packet_type == 4: 
            self.has_class_ID = (byte0 >>3 ) & 0x01
            self.tsi = (byte1 >>6 ) & 0x03
            self.tsf = (byte1 >>4 ) & 0x03
            self.packet_count = byte1  & 0x0F
      
        
        
        

# Full functionality is tested via end-to-end testing using SinkVITA49 and SourceVITA49 in the fulltest_VITA49.py file
class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in rh.SinkVITA49.
    """

    ###################
    #  SETUP/TEARDOWN
    ###################

    def setUp(self):
        """Set up the unit test - this is run before every method that starts with test.
        """
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        
        #######################################################################
        # Launch and initialize the component with the default execparam/property values,
        # and set debug level
        print ' * Note:'
        print ' * During this unit test, SinkVITA49 will warn about using a built-in table'
        print ' * for leap seconds if no leap seconds file is found at the indicated location.'
        self.launch(execparams={"DEBUG_LEVEL": DEBUG_LEVEL})
        
        #######################################################################
        # Set up data input/output and tracking variables
        self.dataSource = sb.DataSource()
        self.dataSource.connect(self.comp, 'dataShort_in')
        self.inVitaPort = bulkio.InVITA49Port("dataVITA49_in")
        self.inVitaPort.setNewAttachDetachListener(self)
        self.sock = None
        self.attaches = 0
        self.detaches = 0


    def tearDown(self):
        """Finish the unit test - this is run after every method that starts with test.
        """
        
        if self.detaches < self.attaches:
            detaches = self.detaches
            self.disconnectVitaPorts()
            self.waitForDetach(previousDetaches=detaches)
        
        #######################################################################
        # Clean up data input/output
        self.inVitaPort._this()._release()
        self.inVitaPort = None
        self.dataSource.releaseObject()
        self.dataSource = None
        self.closeSocket()
        #######################################################################
        # Simulate regular component shutdown
        self.comp.releaseObject()
           
        ossie.utils.testing.ScaComponentTestCase.tearDown(self)

    ###################
    #     HELPERS
    ###################
    
    def callStart(self):
        """ Call start() on the rh.SinkVITA49 component and the test data source object.
        """
        self.comp.start()
        self.dataSource.start()
    
    def callStop(self):
        """ Call stop() on the rh.SinkVITA49 component and the test data source object.
        """
        self.dataSource.stop()
        self.comp.stop()
    
    def connectVitaPorts(self,connId='connId'):
        """ Connect the rh.SinkVITA49 output VITA49 port to the test VITA49 input port.
        """
        outPort = self.comp.getPort("dataVITA49_out")
        outPort.connectPort(self.inVitaPort._this(), connId)
    
    def disconnectVitaPorts(self,connId='connId'):
        """ Disconnect the rh.SinkVITA49 output VITA49 port from the test VITA49 input port.
        """
        outPort = self.comp.getPort("dataVITA49_out")
        outPort.disconnectPort(connId)

    def attach(self, streamDef, userId):
        """ VITA49 attach listener for self.inVitaPort.
        """
        self.attaches += 1
        print "Attaches:", self.attaches, "  UserId:", userId, "  StreamDef:", streamDef

    def detach(self, id):
        """ VITA49 detach listener for self.inVitaPort.
        """
        self.detaches += 1
        print "Detaches:", self.detaches, "  DetachId:", id

    def waitForAttach(self, timeOut=5, waitInterval=0.2, previousAttaches=0):
        """ Wait for VITA49 connection to trigger attach callback.
        """
        runTime = 0
        while timeOut > runTime and self.attaches <= previousAttaches:
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertEqual(self.attaches, previousAttaches+1)

    def waitForDetach(self, timeOut=5, waitInterval=0.2, previousDetaches=0):
        """ Wait for VITA49 disconnection to trigger detach callback.
        """
        runTime = 0
        while timeOut > runTime and self.detaches <= previousDetaches:
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertEqual(self.detaches, previousDetaches+1)
        
    def configureNetwork(self, ip='127.0.0.1', port=24967, vlan=0, iface='lo', udp=True, en=True):
        """ Configure rh.SinkVITA49 network properties
        """
        self.comp.network_settings.ip_address = ip
        self.comp.network_settings.port = port
        self.comp.network_settings.vlan = vlan
        self.comp.network_settings.interface = iface
        self.comp.network_settings.use_udp_protocol = udp
        self.comp.network_settings.enable = en
        
    def configureAdvanced(self, max_payload_size=1452, number_of_buffers=10, force_transmit=False,
                          endian_representation=0, use_bulkio_sri=False, time_between_context_packets=1,
                          number_of_packets_in_burst=150, throttle_time_between_packet_bursts=100):
        """ Configure rh.SinkVITA49 advanced properties
        """
        self.comp.advanced_configuration.max_payload_size = max_payload_size #1452 for MTU of 1500
        self.comp.advanced_configuration.number_of_buffers = number_of_buffers # 10 is the default
        self.comp.advanced_configuration.force_transmit = force_transmit
        self.comp.advanced_configuration.endian_representation = endian_representation
        self.comp.advanced_configuration.use_bulkio_sri = use_bulkio_sri
        self.comp.advanced_configuration.time_between_context_packets = time_between_context_packets
        self.comp.advanced_configuration.number_of_packets_in_burst = number_of_packets_in_burst
        self.comp.advanced_configuration.throttle_time_between_packet_bursts = throttle_time_between_packet_bursts
        
    def createKeywords(self, colBW=20000000, colRF=155500000):
        """ Return list of all VITA49 expected keywords.
        """
        defaultKeywords = []
        defaultKeywords.append(sb.SRIKeyword(name="COL_BW",value=colBW,format="double")) #20000000
        defaultKeywords.append(sb.SRIKeyword(name="COL_RF",value=colRF,format="double")) #155500000
        defaultKeywords.append(sb.SRIKeyword(name="COL_RF_OFFSET",value=20,format="double")) #20
        defaultKeywords.append(sb.SRIKeyword(name="COL_IF",value=70000000,format="double")) #70000000
        defaultKeywords.append(sb.SRIKeyword(name="COL_IF_FREQUENCY_OFFSET",value=30999,format="double")) #30999
        defaultKeywords.append(sb.SRIKeyword(name="COL_REFERENCE_LEVEL", value=10.2969,format="float")) #10.2969
        defaultKeywords.append(sb.SRIKeyword(name="COL_GAIN",value=10,format="float")) #10
        defaultKeywords.append(sb.SRIKeyword(name="DATA_GAIN",value=0,format="float")) #0
        defaultKeywords.append(sb.SRIKeyword(name="ATTENUATION_SUM",value=10,format="float")) #10
        defaultKeywords.append(sb.SRIKeyword(name="OVER_RANGE_SUM",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="TIMESTAMP_ADJUSTMENT",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="TIMESTAMP_CALIBRATION",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="TEMPERATURE",value=100,format="float")) #100
        defaultKeywords.append(sb.SRIKeyword(name="DEVICE_IDENTIFIER",value=4368,format="long")) #4368
        defaultKeywords.append(sb.SRIKeyword(name="CALIBRATED_TIME_STAMP",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="REFERENCE_LOCKED",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="AUTO_GAIN_CONTROL",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="SIGNAL_DETECTION",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="DATA_INVERSION_FLAG",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="OVER_RANGE",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="SAMPLE_LOSS_INDICATOR",value=True,format="boolean")) #True
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.TIME_SECONDS",value=1029071060,format="long")) #1029071060
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.TIME_FRACTIONAL_SECONDS",value=-1554051584,format="long")) #-1554051584
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.MANUFACTURER_ID",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.LAT",value=39.1222,format="double")) #39.1222
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.LON",value=-76.772,format="double")) #-76.772
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.ALT",value=225,format="double")) #225
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.GROUND_SPEED",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.HEADING_ANGLE",value=10,format="double")) #10
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.TRACKING_ANGLE",value=14,format="double")) #14
        defaultKeywords.append(sb.SRIKeyword(name="GEO_GPS.MAGNETIC_VARIATION",value=1,format="double")) #1
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.TIME_SECONDS",value=1029071060,format="long")) #1029071060
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.TIME_FRACTIONAL_SECONDS",value=-1554051584,format="long")) #-1554051584
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.MANUFACTURER_ID",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.LAT",value=3.12223,format="double")) #3.12223
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.LON",value=-16.772,format="double")) #-16.772
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.ALT",value=990,format="double")) #990
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.GROUND_SPEED",value=1,format="double")) #1
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.HEADING_ANGLE",value=87,format="double")) #87
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.TRACKING_ANGLE",value=39,format="double")) #39
        defaultKeywords.append(sb.SRIKeyword(name="GEO_INS.MAGNETIC_VARIATION",value=2,format="double")) #2
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.TIME_SECONDS",value=1029071060,format="long")) #1029071060
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.TIME_FRACTIONAL_SECONDS",value=-1554051584,format="long")) #-1554051584
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.MANUFACTURER_ID",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.POS_X",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.POS_Y",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.POS_Z",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.ALT_ALPHA",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.ALT_BETA",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.ALT_PHI",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.VEL_X",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.VEL_Y",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_ECEF.VEL_Z",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.TIME_SECONDS",value=1029071060,format="long")) #1029071060
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.TIME_FRACTIONAL_SECONDS",value=-1554051584,format="long")) #-1554051584
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.MANUFACTURER_ID",value=0,format="long")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.POS_X",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.POS_Y",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.POS_Z",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.ALT_ALPHA",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.ALT_BETA",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.ALT_PHI",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.VEL_X",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.VEL_Y",value=0,format="double")) #0
        defaultKeywords.append(sb.SRIKeyword(name="EPH_RELATIVE.VEL_Z",value=0,format="double")) #0
        return defaultKeywords

    def validateSRI(self, sri, streamID, xstart = 0,xdelta=.0001,xunits=0,subsize=0,mode=0):
        self.assertEqual(sri.streamID, streamID)        
        self.assertEqual(sri.xstart, xstart)
        self.assertEqual(sri.xdelta, xdelta)
        self.assertEqual(sri.xunits, xunits)
        self.assertEqual(sri.subsize, subsize)
        self.assertEqual(sri.mode, mode)
    
    def validateStreamDef(self, streamDef, streamId, ip='127.0.0.1', vlan=0, port=24967, udp=None):
        """ Validate StreamDef
            For udp, the following values are accepted:
                None - indicates to not validate transport protocol
                bulkio.BULKIO.VITA49_TCP_TRANSPORT
                bulkio.BULKIO.VITA49_UDP_TRANSPORT
                bulkio.BULKIO.VITA49_UNKNOWN_TRANSPORT
        """
        self.assertEqual(streamDef.ip_address,ip)
        self.assertEqual(streamDef.vlan,vlan)
        self.assertEqual(streamDef.port,port)
        self.assertEqual(streamDef.id,streamId)
        if udp != None:
            self.assertEqual(streamDef.protocol,udp)
        
    def setupSocket(self):
        port = 24967
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("", port))
        
    def closeSocket(self):
        try:
            self.sock.close()
        except AttributeError:
            pass
        self.sock = None

    # Same implementation that C++ Boost uses for hashing a std::string. This is what the component uses to go from the string stream ID to the VITA49 numberic stream ID
    def boosthashStr(self,inputStr):
      seed = 0
      for c in inputStr:
        seed ^= (ord(c) + 0x9e3779b9 + (seed<<6) + (seed>>2)) & 0xffffffffffffffff
      return seed & 0xffffffff
    
    # Takes the 64 bit two word number and converts to a floating point based on radix
    def to_double64(self,data,radixPoint):
        divisor = 2**radixPoint
        if divisor !=0:
            return data/divisor
        else:
            return 0


    def validateSocketData(self, dataIn,streamID,complex=False,data_type=3,bw=0,rfreq=0,ifreq=0,sr=0):
        try:
            msg = ''
            count=100
            while len(msg) < 1000 and count > 0:
                data, addr = self.sock.recvfrom(1024)
                if len(data)>0:
                    msg = '%s%s'%(msg,data)
                    print 'Received:',len(data),' Total:',len(msg)
                else:
                    count-=1
                    time.sleep(0.1)
                    
        except:
            pass
        finally:
            HDRLEN=32
            msg = msg.split('VRLP')
            for m in msg:
              if len(m) > 32:
                header = vita49_data_header(m[:HDRLEN])
                #print "Type " , header.packet_type, len(m)
                if header.packet_type ==1: #Data Packet
                    # Validate Header
                    header = vita49_data_header(m[:HDRLEN])
                    id = self.boosthashStr(streamID)
                    self.assertEqual(header.streamID,id)
                    self.assertEqual(header.complex, complex)
                    self.assertEqual(header.data_type,data_type)
                    m=m[HDRLEN:].split('VEND')[0]
                    fmt = 'h'*int(len(m)/2)
                    m1 = struct.unpack(fmt,m)
                    self.assertEqual(list(m1),dataIn[:len(m1)])

                if header.packet_type ==4: #Context Packet        
                    header = vita49_data_header(m[:HDRLEN])
                    id = self.boosthashStr(streamID)
                    self.assertEqual(header.streamID,id)
                    m=m[HDRLEN:].split('VEND')[0]
                    #fmt = '!'+'I'*int(len(m)/4)
                    contextpayload=struct.unpack("!I",m[:4])
                    m = m[4:]
                    
                    if (contextpayload[0] >>30 ) & 0x0001:  #Reference Point Identifier
                        m = m[4:]
                    if (contextpayload[0] >>29 ) & 0x0001:  #Bandwidth
                        bw_context = self.to_double64(struct.unpack("!q",m[:8])[0],20)
                        self.assertEqual(bw,bw_context)
                        m = m[8:]        
                    if (contextpayload[0] >>28 ) & 0x0001:  #IF Freq
                        if_context = self.to_double64(struct.unpack("!q",m[:8])[0],20)
                        self.assertEqual(ifreq,if_context)
                        m = m[8:]      
                    if (contextpayload[0] >>27 ) & 0x0001:  #RF Freq
                        rf_context = self.to_double64(struct.unpack("!q",m[:8])[0],20)
                        self.assertEqual(rfreq,rf_context)
                        m = m[8:]         
                    if (contextpayload[0] >>26 ) & 0x0001:  #RF Freq Offset
                        m = m[8:]  
                    if (contextpayload[0] >>25 ) & 0x0001:  #IF Band Offset
                        m = m[8:] 
                    if (contextpayload[0] >>24 ) & 0x0001:  #Reference Level
                        m = m[4:] 
                    if (contextpayload[0] >>23 ) & 0x0001:  #Gain
                        m = m[4:] 
                    if (contextpayload[0] >>22 ) & 0x0001:  #Over-range Count
                        m = m[4:] 
                    if (contextpayload[0] >>21 ) & 0x0001:  #Sample Rate
                        sr_context = self.to_double64(struct.unpack("!q",m[:8])[0],20)
                        self.assertEqual(sr, sr_context)
                        m = m[8:] 
   
    ###################
    #   BEGIN TESTS
    ###################

    def testScaBasicBehavior(self):
        """testScaBasicBehavior
        """
        #######################################################################
        # Verify the basic state of the resource
        self.assertNotEqual(self.comp, None)
        self.assertEqual(self.comp.ref._non_existent(), False)
        self.assertEqual(self.comp.ref._is_a("IDL:CF/Resource:1.0"), True)

        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)

        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)

        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
    
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()


    def testAttachmentSendsStreamDef(self):
        """testAttachmentSendsStreamDef
        """
        # Configure network info
        self.configureNetwork()
        
        # Start components
        self.callStart()
        
        streamId = "testAttachmentSendsStreamDef"
        dataIn = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(dataIn, streamID=streamId, sampleRate=10000.0)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.validateStreamDef(recvStreamDef, streamId)


    def testSendData(self):
        """testSendData
        """
        # Configure network info
        self.configureNetwork()
        
        # Set up receiver
        self.setupSocket()
        #print "waiting on port:", port
        
        # Start components
        self.callStart()
        
        streamId = "testSendData"
        dataIn = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        keywords = [sb.SRIKeyword("COL_BW",30.0,"double"),sb.SRIKeyword("COL_RF",150000000.0,"double")]
        self.dataSource.push(dataIn, streamID=streamId, sampleRate=10000.0,SRIKeywords=keywords )
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.validateStreamDef(recvStreamDef, streamId)
        
        # End the stream so the remaining data is pushed
        self.dataSource.push([], EOS=True, streamID=streamId, sampleRate=10000.0)
        
        # Get the data and validate
        #Input data was real short data 
        self.validateSocketData(dataIn,streamId,False,3,bw=30.0,rfreq=150000000.0,ifreq=10000.0/4,sr=10000.0)
        self.closeSocket()

    def testSendDataComplex(self):
        """testSendData
        """
        # Configure network info
        self.configureNetwork()
        
        # Set up receiver
        self.setupSocket()
        #print "waiting on port:", port
        
        # Start components
        self.callStart()
        
        streamId = "testSendDataComplex"
        dataIn = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        keywords = [sb.SRIKeyword("COL_BW",30.0,"double"),sb.SRIKeyword("COL_RF",150000000.0,"double")]
        self.dataSource.push(dataIn, streamID=streamId, sampleRate=10000.0,complexData=True,SRIKeywords=keywords )
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.validateStreamDef(recvStreamDef, streamId)
        
        # End the stream so the remaining data is pushed
        self.dataSource.push([], EOS=True, streamID=streamId, sampleRate=10000.0)
        
        # Get the data and validate
        #Input data was real short data 
        self.validateSocketData(dataIn,streamId,True,3,bw=30.0,rfreq=150000000.0,ifreq=0,sr=10000.0)
        self.closeSocket()

    def testSRIValid(self):
        """testSendData
        """
        # Configure network info
        self.configureNetwork()
        
        # Set up receiver
        #self.setupSocket()
        #print "waiting on port:", port
        
        # Start components
        self.callStart()
        
        streamId = "testSRIValid"
        dataIn = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(dataIn, streamID=streamId, sampleRate=10000.0)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.validateStreamDef(recvStreamDef, streamId)
        
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId)
        # Push more data with new SR and check SRI update
        self.dataSource.push(dataIn, EOS=True,streamID=streamId, sampleRate=50000.0)
        detaches=self.detaches
        self.waitForDetach(previousDetaches=detaches)
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId,xdelta=1.0/50000.0)
        

        
        # Change StreamID and push data and confirm new SRI is sent. 
        time.sleep(0.1)
        attaches=self.attaches
        streamId = "testSRIValid2"
        self.dataSource.push(dataIn, streamID=streamId, sampleRate=20000.0)
        self.waitForAttach(previousAttaches=attaches)
        time.sleep(0.1)
        
        #Check Stream Def
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)
        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        self.validateStreamDef(recvStreamDef, streamId)
        
        #check SRI
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId,xdelta=1.0/20000.0)
        
    def testDetachmentOnDisconnect(self):
        """testDetachmentOnDisconnect
        """
        # Configure network info
        self.configureNetwork()
        
        # Start components
        self.callStart()
        
        streamId = "testDetachmentOnDisconnect"
        data = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(data,streamID=streamId)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)
        
        detaches=self.detaches
        self.disconnectVitaPorts()
        self.waitForDetach(previousDetaches=detaches)
        
        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),0)

    def testReconnect(self):
        """testReconnect
        """
        # Configure network info
        self.configureNetwork()
        
        # Start components
        self.callStart()
        
        streamId = "testReconnect"
        data = range(1000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(data,streamID=streamId)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)
        
        detaches=self.detaches
        self.disconnectVitaPorts()
        self.waitForDetach(previousDetaches=detaches)
        
        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),0)
        
        attaches=self.attaches
        self.connectVitaPorts()
        self.waitForAttach(previousAttaches=attaches)
        
        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)
        
        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.assertEqual(recvStreamDef.id, streamId)


    def testSRIKeywords(self):
        """testSRIKeywords
        """
        # Configure network info
        self.configureNetwork()
    
        # Start components
        self.callStart()
    
        # Push SRI and data
        streamId = "testSRIKeywords"
        data = range(10000)
        defaultKeywords = self.createKeywords()
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(data, streamID=streamId, sampleRate=10000.0, SRIKeywords=defaultKeywords)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.assertEqual(recvStreamDef.id, streamId)
        
        #check SRI
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId)
        
        # TODO - more validation


    def testDataEOS(self):
        """testDataEOS
        """
        # Configure network info
        self.configureNetwork()
    
        # Start components
        self.callStart()
    
        # Push SRI and data
        streamId = "testDataEOS"
        data = range(10000)
        attaches=self.attaches
        detaches=self.detaches
        self.connectVitaPorts()
        self.dataSource.push(data, EOS=True, streamID=streamId,sampleRate=10000.0)
        self.waitForAttach(previousAttaches=attaches)
        self.waitForDetach(previousDetaches=detaches)
        
        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),0)
        
        #check SRI
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId)
        
        # TODO - more validation


    def testDataComplex(self):
        """testDataComplex
        """
        # Configure network info
        self.configureNetwork()
    
        # Start components
        self.callStart()
    
        # Push SRI and data
        streamId = "testDataComplex"
        data = range(10000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(data, streamID=streamId, sampleRate=10000.0, complexData=True)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.assertEqual(recvStreamDef.id, streamId)
        #check SRI
        self.assertEqual(self.inVitaPort.sriDict.has_key(streamId),True)
        sri = self.inVitaPort.sriDict[streamId][0]
        self.validateSRI(sri,streamId,mode=1)
        

    def testForceTransmit(self):
        """testForceTransmit
        """
        # Configure network info
        self.configureNetwork()
        
        # Configure advanced properties
        self.configureAdvanced(force_transmit=True)
        
        # Start components
        self.callStart()
    
        # Push SRI and data
        streamId = "testForceTransmit"
        data = range(10000)
        attaches=self.attaches
        self.connectVitaPorts()
        self.dataSource.push(data, streamID=streamId, sampleRate=10000.0)
        self.waitForAttach(previousAttaches=attaches)

        time.sleep(0.1) # This is necessary b/c it can take the port some time to update
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)
        
        self.assertEqual(recvStreamDef.id, streamId)
        
        # TODO - more validation

if __name__ == "__main__":
    ossie.utils.testing.main("../SinkVITA49.spd.xml") # By default tests all implementations
