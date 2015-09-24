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

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in SinkVITA49"""

    ###################
    #  SETUP/TEARDOWN
    ###################
    
    def setUp(self):
        self.dataSource = sb.DataSource()
        self.comp = sb.launch("../SinkVITA49.spd.xml")
        self.inVitaPort = bulkio.InVITA49Port("dataVITA49_in")
        self.dataSource.connect(self.comp, 'dataShort_in')
        self.inVitaPort.setNewAttachDetachListener(self)

        self.attaches = 0
        self.detaches = 0

    def tearDown(self):
        self.inVitaPort._this()._release()
        self.inVitaPort = None
        self.comp.releaseObject()
        self.comp = None
        self.dataSource = None


    ###################
    #     HELPERS
    ###################


    def connectVitaPorts(self,connId='connId'):
        outPort = self.comp.getPort("dataVITA49_out")
        outPort.connectPort(self.inVitaPort._this(), connId)
    
    def disconnectVitaPorts(self,connId='connId'):
        outPort = self.comp.getPort("dataVITA49_out")
        outPort.disconnectPort(connId)

    def attach(self, streamDef, userId):
        self.attaches += 1
        #print "StreamDef: " + str(streamDef)

    def detach(self, id):
        self.detaches += 1
        #print "DetachId: " + str(id)

    def waitForAttach(self, timeOut=5, waitInterval=0.2):
        self.attaches = 0
        runTime = 0
        while timeOut > runTime:
            if self.attaches > 0:
                self.assertNotEqual(self.attaches, 0)
                return
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertNotEqual(self.attaches, 0)

    def waitForDetach(self, timeOut=5, waitInterval=0.2):
        self.deattaches = 0
        runTime = 0
        while timeOut > runTime:
            if self.detaches > 0:
                self.assertNotEqual(self.detaches, 0)
                return
            time.sleep(waitInterval)
            runTime += waitInterval
        self.assertNotEqual(self.detaches, 0)


    ###################
    #   BEGIN TESTS
    ###################


    def testAttachmentSendsStreamDef(self):
        streamId = "SomeStreamId"
        self.dataSource.start()
        self.comp.start()
        data = range(1000)
        self.connectVitaPorts()
        self.dataSource.push(data,EOS=False,streamID=streamId)
        self.waitForAttach()

        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)

        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)
        self.assertEqual(recvStreamDef.id, streamId)

    def testDetachmentOnDisconnect(self):
        streamId = "SomeStreamId"
        self.dataSource.start()
        self.comp.start()
        data = range(1000)
        self.connectVitaPorts()
        self.dataSource.push(data,EOS=False,streamID=streamId)
        self.waitForAttach()
        self.disconnectVitaPorts()
        self.waitForDetach()
        
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),0)

    def testReconnect(self):
        streamId = "SomeStreamId"
        self.dataSource.start()
        self.comp.start()
        data = range(1000)
        self.connectVitaPorts()
        self.dataSource.push(data,EOS=False,streamID=streamId)
        self.waitForAttach()
        self.disconnectVitaPorts()
        self.waitForDetach()
        self.connectVitaPorts()
        
        attachId = self.inVitaPort._get_attachmentIds()[0]
        recvStreamDef = self.inVitaPort.getStreamDefinition(attachId)

        self.assertEqual(recvStreamDef.id, streamId)
        self.assertEqual(len(self.inVitaPort._get_attachmentIds()),1)

    def testForceTransmit(self):
        self.comp.advanced_configuration.force_transmit = True
        self.comp.start()
        


    def ntest4(self):
        pass

    def ntest5(self):
        pass

if __name__ == "__main__":
    ossie.utils.testing.main("../SinkVITA49.spd.xml") # By default tests all implementations
