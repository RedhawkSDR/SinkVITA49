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
import unittest
import ossie.utils.testing
import os
from omniORB import any, CORBA
from ossie.cf import CF
from ossie.utils.bulkio import bulkio_data_helpers
import bulkio
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
from ossie.utils import sb
import time
import sys

from ossie.utils.sandbox import debugger
import netifaces # sudo yum install python-netifaces

SINK_DEBUG_LEVEL=3
SOURCE_DEBUG_LEVEL=3

# Attempt to determine local nic interface
NIC_INTERFACES=['eth0','eth1','eth2','em0','em1','em2']
INTERFACE_INTERSECTION=list(set(netifaces.interfaces()) & set(NIC_INTERFACES))
NIC_INTERFACE="eth0"
if len(INTERFACE_INTERSECTION) > 0: NIC_INTERFACE=INTERFACE_INTERSECTION[0]

# Attempt to determine local nic IP address
IP_ADDRESS="127.0.0.1"
try:
    ifaddress = netifaces.ifaddresses(NIC_INTERFACE)
    IP_ADDRESS = ifaddress[2][0]['addr']
except:
    print "Unable to determine IP address for '" + NIC_INTERFACE + "'"

# Status of testing parameters
print "   #######################################"
print "   # TESTING SETTINGS "
print "   #######################################"
print "   #      NIC INTRFC: " + NIC_INTERFACE
print "   #      IP ADDRESS: " + IP_ADDRESS
print "   #######################################"

# Create a VALGRIND debugger for more detail
myDebugger = debugger.Valgrind()
myDebugger.arguments = ["--tool=memcheck","--track-origins=yes"]

SHORT_PORT='short'
USHORT_PORT='ushort'
DOUBLE_PORT='double'
FLOAT_PORT='float'
OCTET_PORT='octet'
CHAR_PORT='char'

SHORT_DATA=[-2,-1,0,1,2] * 100000
USHORT_DATA=[0,1,2,3,4] * 100000
DOUBLE_DATA=[0.1, 0.5, 0.4, 10.4] * 100000
FLOAT_DATA=DOUBLE_DATA
OCTET_DATA=[1,2,3,4,5] * 100000
CHAR_DATA=[1,2,3,4,5] * 100000

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in SinkVITA49"""

    ###################
    #  SETUP/TEARDOWN
    ###################
    
    def setUp(self):
        self.dataSource = sb.DataSource()
        self.dataSource._sampleRate = 8000
        self.dataSink = sb.DataSink()
        self.sink = sb.launch("SinkVITA49", execparams={"DEBUG_LEVEL":SINK_DEBUG_LEVEL})#, debugger=myDebugger)
        self.source = sb.launch("SourceVITA49", execparams={"DEBUG_LEVEL":SOURCE_DEBUG_LEVEL})#, debugger=myDebugger)

        # Setup sink/source settings
        self.sink.network_settings.enable = True
        self.sink.network_settings.ip_address = IP_ADDRESS
        self.sink.network_settings.interface = NIC_INTERFACE
        self.source.interface = NIC_INTERFACE

        self.attaches = 0
        self.detaches = 0

    def tearDown(self):
        self.dataSource.stop()
        self.source.stop()
        self.sink.stop()
        self.sink.releaseObject()
        self.source.releaseObject()
        self.sink = None
        self.source = None
        self.dataSource = None


    ###################
    #     HELPERS
    ###################

    def connectAllPorts(self, dataPort):
        self.connectVitaPorts()
        self.connectDataFlowPorts(dataPort=dataPort)

    def connectVitaPorts(self,connId='connId'):
        self.sink.connect(self.source, "dataVITA49_in", connectionId=connId)

    def connectDataFlowPorts(self, dataPort):
        vitaDataPortName = "data"+dataPort.title()+"_in"
        dataSinkPortName = dataPort.lower()+"In"
        self.source.connect(self.dataSink, dataSinkPortName)
        self.dataSource.connect(self.sink, vitaDataPortName)
    
    def disconnectAllPorts(self):
        self.disconnectVitaPorts()
        self.disconnectDataFlowPorts()
    
    def disconnectVitaPorts(self, connId='connId'):
        self.vitaOutPort().disconnectPort(connId)
    
    def disconnectDataFlowPorts(self):
        self.dataSink.disconnect(self.source)
        self.sink.disconnect(self.dataSource)
    
    def vitaInPort(self):
        return self.source.getPort('dataVITA49_in')
    
    def vitaOutPort(self):
        return self.sink.getPort('dataVITA49_out')

    def pushDataAndWait(self, data, streamID, repeatCount=3, timeout=2, EOS=False):

        # Last attempt should be asserted to throw error
        for n in range(repeatCount-1):
            self.dataSource.push(data,EOS,streamID)
            if self.waitForData(timeout/repeatCount): 
                 return # Data was received
        self.dataSource.push(data,EOS,streamID)
        self.assertWaitForData(timeout/repeatCount)
        
    
    def setupStream(self, streamId, dataPortName):
        # Start the sink/source components
        self.dataSource.start()
        self.dataSink.start()
        self.connectAllPorts(dataPort=dataPortName)
        self.sink.start()

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L
        
        # Valid SRI required before stream setup - Trigger SRI push
        self.dataSource.push([],EOS=False,streamID=streamId, sampleRate=8000)
        
        # Wait for the stream to be created/attached
        self.waitForAttach()
        self.assertAttachmentExists()
        
        # Start source AFTER attach so that stream port is active
        self.source.start()

        # Wait one second to gaurentee context packet is received
        self.waitForClientSetup() 
        
    def cleanupStream(self, streamId):
        self.dataSource.push([],EOS=True,streamID=streamId)
        self.source.stop() # To remove annoying invalid attach warning
        self.disconnectAllPorts()
        self.waitForDetach()

    ###################
    #  WAIT HELPERS
    ###################


    def waitForAttach(self, expectedAttaches=1, timeOut=5, waitInterval=0.2):
        self.attaches = 0
        runTime = 0
        while timeOut > runTime:
            attachments = self.vitaInPort()._get_attachmentIds()
            if len(attachments) >= expectedAttaches:
                return
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertTrue(False) # Failed to attach

    def waitForDetach(self, expectedAttaches=0, timeOut=5, waitInterval=0.2):
        self.deattaches = 0
        runTime = 0
        while timeOut > runTime:
            attachments = self.vitaInPort()._get_attachmentIds()
            if len(self.vitaInPort()._get_attachmentIds()) <= expectedAttaches:
                return
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertTrue(False) # Failed to detach
    
    def waitForData(self, timeOut=5, waitInterval=0.2):
        self.deattaches = 0
        runTime = 0
        while timeOut > runTime:
            if len(self.dataSink.getData()) > 0:
                return True
            time.sleep(waitInterval)
            runTime += waitInterval
        return False
    
    def assertWaitForData(self, timeOut=5, waitInterval=0.2):
        self.deattaches = 0
        runTime = 0
        while timeOut > runTime:
            if len(self.dataSink.getData()) > 0:
                return
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertNotEqual(len(self.dataSink.getData()), 0, "Timed out waiting for data")

    def waitForClientSetup(self, timer=1.5):
        time.sleep(timer)


    ###################
    #  ASSERT HELPERS
    ###################
    
    def assertAttachmentExists(self):
        self.assertTrue(len(self.vitaInPort()._get_attachmentIds()) > 0)


    ###################
    # BEGIN PORT TESTS
    ###################
    
    def portStartedConnectedWithInvalidInterface(self, portType, dataSample):
        streamId = "someStream"
        correctNic = self.source.interface
        self.source.interface = "WRONG"

        self.setupStream(streamId, portType)

        # Valid SRI required before stream setup - Trigger SRI push
        self.dataSource.push([],EOS=False,streamID=streamId, sampleRate=8000)
        
        # Start Data Sink/Source
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.source.start()
        
        # Connect ports
        self.connectDataFlowPorts(portType)
        self.connectVitaPorts()
        
        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L
        self.source.interface = NIC_INTERFACE

        # Push data and give time to process
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    
    def portConnectedStartedWithInvalidInterface(self, portType, dataSample):
        streamId = "someStream"
        correctNic = self.source.interface
        self.source.interface = "WRONG"

        self.setupStream(streamId, portType)

        # Valid SRI required before stream setup - Trigger SRI push
        self.dataSource.push([],EOS=False,streamID=streamId, sampleRate=8000)
        
        # Connect ports
        self.disconnectVitaPorts()
        self.connectVitaPorts()
        
        # Start Data Sink/Source
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.source.start()
        
        # Connect ports
        self.disconnectVitaPorts()
        self.connectVitaPorts()

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L
        self.source.interface = NIC_INTERFACE

        # Push data and give time to process
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
       
        self.cleanupStream(streamId)

    def portTestConnectBeforeStarted(self, portType, dataSample):
        streamId = "someStream"
        
        self.setupStream(streamId, portType)

        # Connect ports
        self.disconnectVitaPorts()
        self.connectVitaPorts()
        
        # Start Data Sink/Source
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.source.start()

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L

        # Push data
        data = dataSample
        self.pushDataAndWait(data,repeatCount=5,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestConnectAfterStarted(self, portType, dataSample):
        streamId = "someStream"
        
        self.setupStream(streamId, portType)
        self.disconnectVitaPorts()

        # Start Data Sink/Source
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.source.start()
        
        # Connect ports
        self.connectVitaPorts()

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L

        # Push data and give time to process
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestAttachmentSendsData(self, portType, dataSample):
        streamId = "someStream"

        self.setupStream(streamId, portType)

        # Push data and give time to process
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestSRIPushing(self, portType, dataSample):
        stream1 = "Stream-1"
        stream2 = "Stream-2"
        stream3 = "Stream-3"
        
        # Check initial SRI push
        self.setupStream(stream1, portType)
        self.assertEqual(self.dataSink.sri().streamID, stream1)

        # Check secondary SRI push with first stream closed
        self.dataSource.push([],EOS=True,streamID=stream1)
        self.dataSource.push([],EOS=False,streamID=stream2)
        time.sleep(2)
        self.assertEqual(self.dataSink.sri().streamID, stream2)

        # Check SRI push with stream3 doesn't clobber stream 2
        self.dataSource.push([],EOS=False,streamID=stream3)
        self.assertEqual(self.dataSink.sri().streamID, stream2)
        
        self.cleanupStream(stream2)
        self.cleanupStream(stream3)

    def portTestAttachmentChangePortBeforeRunning(self, portType, dataSample):
        self.sink.network_settings.port = 12346
        
        # Setup stream
        streamId = "someStream"
        self.setupStream(streamId, portType)

        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestAttachmentChangePortWhileRunning(self, portType, dataSample):
        # Setup stream
        streamId = "someStream"
        self.setupStream(streamId, portType)

        # Change port
        self.sink.network_settings.port = 12346L

        # Wait for context packet to be sent/processed
        self.waitForClientSetup(5)

        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=5,timeout=10,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestAttachmentChangeIPBeforeRunning(self, portType, dataSample):
        self.sink.network_settings.ip_address = "127.0.0.1"
        
        # Setup stream
        streamId = "someStream"
        self.setupStream(streamId, portType)

        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestAttachmentChangeIPWhileRunning(self, portType, dataSample):
        # Setup stream
        streamId = "someStream"
        self.setupStream(streamId, portType)

        # Change port
        self.sink.network_settings.ip_address = "127.0.0.1"
        self.waitForClientSetup()

        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestForcedTransmit(self, portType, dataSample):
        streamId = "someStream"
       
        # Setup the stream
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.connectDataFlowPorts(portType)

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L
        
        # Valid SRI required before stream setup - Trigger SRI push
        self.dataSource.push([],EOS=False,streamID=streamId, sampleRate=8000)
        
        # Setup forced transmit 
        self.sink.advanced_configuration.force_transmit = True
        self.source.attachment_override.ip_address = IP_ADDRESS
        self.source.attachment_override.port = 12344L
        self.source.attachment_override.enabled = True
        
        # unicast client doesn't work unless we give sink enough time to do ???
        self.waitForClientSetup()
        
        # Start source AFTER attach so that stream port is active
        self.source.start()
        
        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)


    def portTestReconnectVitaPorts(self, portType, dataSample):
        streamId = "someStream"
        self.setupStream(streamId, portType)

        self.disconnectVitaPorts()
        self.connectVitaPorts()

        # Wait for new context packet
        self.waitForClientSetup() 

        # Push data and give time to process
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)

    def portTestForcedTransmitEnabledDisabled(self, portType, dataSample):
        streamId = "someStream"
       
        # Setup the stream
        self.sink.network_settings.port = 12346
        self.dataSource.start()
        self.dataSink.start()
        self.sink.start()
        self.connectAllPorts(portType)

        # Transfer size is too large for our testing
        self.source.advanced_configuration.corba_transfersize = 128000L
        
        # Valid SRI required before stream setup - Trigger SRI push
        self.dataSource.push([],EOS=False,streamID=streamId, sampleRate=8000)
        
        # Setup forced transmit 
        self.sink.advanced_configuration.force_transmit = True
        self.source.attachment_override.ip_address = IP_ADDRESS
        self.source.attachment_override.port = 12349L
        self.source.attachment_override.enabled = True
        
        # Start source AFTER attach so that stream port is active
        self.source.start()

        # Wait for the client to be setup
        self.waitForClientSetup() 
        
        # Setup forced transmit 
        self.sink.advanced_configuration.force_transmit = False
        self.source.attachment_override.enabled = False
        
        # Start source AFTER attach so that stream port is active
        self.waitForClientSetup() 
        
        # Push data through
        data = dataSample
        self.pushDataAndWait(data,repeatCount=3,timeout=5,EOS=False,streamID=streamId)
        
        self.cleanupStream(streamId)
    

    # SHORT TESTS 
    def testShortConnectAfterStarted(self):
        self.portTestConnectAfterStarted(SHORT_PORT, SHORT_DATA)
    def testShortConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(SHORT_PORT, SHORT_DATA)
    def testShortConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(SHORT_PORT, SHORT_DATA)
    def testShortStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(SHORT_PORT, SHORT_DATA)
    def testShortAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(SHORT_PORT, SHORT_DATA)
    def testShortSRIPushing(self):          
        self.portTestSRIPushing(SHORT_PORT, SHORT_DATA)
    def testShortAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(SHORT_PORT, SHORT_DATA)
    def testShortAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(SHORT_PORT, SHORT_DATA)
    def testShortAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(SHORT_PORT, SHORT_DATA)
    def testShortAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(SHORT_PORT, SHORT_DATA)
    def testShortForcedTransmit(self):
        self.portTestForcedTransmit(SHORT_PORT, SHORT_DATA)
    def testShortReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(SHORT_PORT, SHORT_DATA)
    def testShortForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(SHORT_PORT, SHORT_DATA)
   
    # USHORT TESTS 
    def testUshortConnectAfterStarted(self):
        self.portTestConnectAfterStarted(USHORT_PORT, USHORT_DATA)
    def testUshortConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(USHORT_PORT, USHORT_DATA)
    def testUshortConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(USHORT_PORT, USHORT_DATA)
    def testUshortStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(USHORT_PORT, USHORT_DATA)
    def testUshortAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(USHORT_PORT, USHORT_DATA)
    def testUshortSRIPushing(self):          
        self.portTestSRIPushing(USHORT_PORT, USHORT_DATA)
    def testUshortAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(USHORT_PORT, USHORT_DATA)
    def testUshortAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(USHORT_PORT, USHORT_DATA)
    def testUshortAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(USHORT_PORT, USHORT_DATA)
    def testUshortAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(USHORT_PORT, USHORT_DATA)
    def testUshortForcedTransmit(self):
        self.portTestForcedTransmit(USHORT_PORT, USHORT_DATA)
    def testUshortReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(USHORT_PORT, USHORT_DATA)
    def testUshortForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(USHORT_PORT, USHORT_DATA)
   
    # FLOAT TESTS 
    def testFloatConnectAfterStarted(self):
        self.portTestConnectAfterStarted(FLOAT_PORT, FLOAT_DATA)
    def testFloatConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(FLOAT_PORT, FLOAT_DATA)
    def testFloatConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(FLOAT_PORT, FLOAT_DATA)
    def testFloatStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(FLOAT_PORT, FLOAT_DATA)
    def testFloatAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(FLOAT_PORT, FLOAT_DATA)
    def testFloatSRIPushing(self):          
        self.portTestSRIPushing(FLOAT_PORT, FLOAT_DATA)
    def testFloatAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(FLOAT_PORT, FLOAT_DATA)
    def testFloatAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(FLOAT_PORT, FLOAT_DATA)
    def testFloatAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(FLOAT_PORT, FLOAT_DATA)
    def testFloatAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(FLOAT_PORT, FLOAT_DATA)
    def testFloatForcedTransmit(self):
        self.portTestForcedTransmit(FLOAT_PORT, FLOAT_DATA)
    def testFloatReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(FLOAT_PORT, FLOAT_DATA)
    def testFloatForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(FLOAT_PORT, FLOAT_DATA)
   
   # DOUBLE TESTS 
    def testDoubleConnectAfterStarted(self):
        self.portTestConnectAfterStarted(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleSRIPushing(self):          
        self.portTestSRIPushing(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleForcedTransmit(self):
        self.portTestForcedTransmit(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(DOUBLE_PORT, DOUBLE_DATA)
    def testDoubleForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(DOUBLE_PORT, DOUBLE_DATA)

    # OCTET
    def testOctetConnectAfterStarted(self):
        self.portTestConnectAfterStarted(OCTET_PORT, OCTET_DATA)
    def testOctetConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(OCTET_PORT, OCTET_DATA)
    def testOctetConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(OCTET_PORT, OCTET_DATA)
    def testOctetStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(OCTET_PORT, OCTET_DATA)
    def testOctetAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(OCTET_PORT, OCTET_DATA)
    def testOctetSRIPushing(self):          
        self.portTestSRIPushing(OCTET_PORT, OCTET_DATA)
    def testOctetAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(OCTET_PORT, OCTET_DATA)
    def testOctetAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(OCTET_PORT, OCTET_DATA)
    def testOctetAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(OCTET_PORT, OCTET_DATA)
    def testOctetAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(OCTET_PORT, OCTET_DATA)
    def testOctetForcedTransmit(self):
        self.portTestForcedTransmit(OCTET_PORT, OCTET_DATA)
    def testOctetReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(OCTET_PORT, OCTET_DATA)
    def testOctetForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(OCTET_PORT, OCTET_DATA)

    # CHAR TESTS 
    def testCharConnectAfterStarted(self):
        self.portTestConnectAfterStarted(CHAR_PORT, CHAR_DATA)
    def testCharConnectBeforeStarted(self):
        self.portTestConnectBeforeStarted(CHAR_PORT, CHAR_DATA)
    def testCharConnectedStartedWithInvalidInterface(self):
        self.portConnectedStartedWithInvalidInterface(CHAR_PORT, CHAR_DATA)
    def testCharStartedConnectedWithInvalidInterface(self):
        self.portStartedConnectedWithInvalidInterface(CHAR_PORT, CHAR_DATA)
    def testCharAttachmentSendsData(self): 
        self.portTestAttachmentSendsData(CHAR_PORT, CHAR_DATA)
    def testCharSRIPushing(self):          
        self.portTestSRIPushing(CHAR_PORT, CHAR_DATA)
    def testCharAttachmentChangePortBeforeRunning(self):
        self.portTestAttachmentChangePortBeforeRunning(CHAR_PORT, CHAR_DATA)
    def testCharAttachmentChangePortWhileRunning(self):
        self.portTestAttachmentChangePortWhileRunning(CHAR_PORT, CHAR_DATA)
    def testCharAttachmentChangeIPBeforeRunning(self):
        self.portTestAttachmentChangeIPBeforeRunning(CHAR_PORT, CHAR_DATA)
    def testCharAttachmentChangeIPWhileRunning(self):
        self.portTestAttachmentChangeIPWhileRunning(CHAR_PORT, CHAR_DATA)
    def testCharForcedTransmit(self):
        self.portTestForcedTransmit(CHAR_PORT, CHAR_DATA)
    def testCharReconnectVitaPorts(self):
        self.portTestReconnectVitaPorts(CHAR_PORT, CHAR_DATA)
    def testCharForcedTransmitEnabledDisabled(self):
        self.portTestForcedTransmitEnabledDisabled(CHAR_PORT, CHAR_DATA)
   
if __name__ == "__main__":
    ossie.utils.testing.main("../SinkVITA49.spd.xml") # By default tests all implementations
