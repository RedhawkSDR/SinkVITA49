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
import os, sys
from omniORB import any, CORBA
from ossie.cf import CF
from ossie.utils.bulkio import bulkio_data_helpers
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
from ossie.utils import sb
 
import time, struct
from ossie import properties

ip_address = ""
ethernet_device = "em1"
network_port = 12345
vlan = 0
f = open('unit_test.log','w')
#f.write('*********************************\n')
#f.write('********** Unit Test 1 **********\n')
#f.write( '*********************************\n\n')
#
#f.write('********** Creating components and connections **********\n')
#sink = sb.Component('SinkVITA49',execparams={"DEBUG_LEVEL":0})
#source = sb.Component('SourceVITA49',execparams={"DEBUG_LEVEL":0})
##push data into the sink NIC
#inputS=sb.DataSource();
#outputS=sb.DataSink();
#
#sink.connect(source, providesPortName='dataVITA49_in')
#inputS.connect(sink,providesPortName='dataShort_in')
#source.connect(outputS,providesPortName='shortIn')
#
#f.write('***** Source Configuration *****\n')
#source.interface = "em1"
#f.write('source.interface = "em1"\n')
#source.attachment_override.enabled = False
#f.write( 'source.attachment_override.enabled = False\n')
#source.advanced_configuration.corba_transfersize = 2928
#f.write( 'source.advanced_configuration.corba_transfersize = 2928\n')
#source.advanced_configuration.bufferSize = 2048
#f.write( 'source.advanced_configuration.bufferSize = 2048\n')
#source.advanced_configuration.poll_in_time = 0
#f.write( 'source.advanced_configuration.poll_in_time = 0\n\n')
#
#f.write('***** Sink Configuration *****\n')
#sink.network_settings.interface = "em1"
#f.write('sink.network_settings.interface = "em1"\n')
#sink.network_settings.use_udp_protocol = True
#f.write('sink.network_settings.use_udp_protocol = True\n')
#sink.network_settings.vlan = 0
#f.write('sink.network_settings.vlan = 0\n')
#sink.network_settings.port = 29465
#f.write('sink.network_settings.port = 29465\n')
#sink.network_settings.address = ip_address
#f.write('sink.network_settings.address = ip_address\n')
#sink.network_settings.enable = True
#f.write('sink.network_settings.enable = True\n')
#sink.numberOfBuffers = 200
#f.write('sink.numberOfBuffers = 200\n')
#sink.advanced_configuration.endian_representation = 2
#f.write('sink.advanced_configuration.endian_representation = 2\n')
#
#
#output_result = sb.FileSink('results.tmp')
#source.connect(output_result,providesPortName='shortIn')
#
#f.write('\n********** Starting Components **********\n\n') 
#sb.start()
#f.write('Components started\n')
#
#input_file = sb.FileSource('golden.tmp', dataFormat='short')
#
#kw = sb.SRIKeyword("COL_BW",20000000.0,'double')
#kw1 = sb.SRIKeyword("COL_IF_FREQUENCY_OFFSET",70000000.0,'double')
#kw2 = sb.SRIKeyword("COL_RF",155500000.0,'double')
#kw3 = sb.SRIKeyword("COL_RF_OFFSET",20.0,'double')
#kw5 = sb.SRIKeyword("COL_REFERENCE_LEVEL",10.2969,'float')
#kw6 = sb.SRIKeyword("COL_GAIN",10.0,'float')
#kw7 = sb.SRIKeyword("DATA_GAIN",0.0,'float')
#kw8 = sb.SRIKeyword("ATTENUATION_SUM",10.0,'float')
#kw9 = sb.SRIKeyword("OVER_RANGE_SUM",0,'long')
#kw10 = sb.SRIKeyword("TIMESTAMP_ADJUSTMENT",0,'long')
#kw11 = sb.SRIKeyword("TIMESTAMP_CALIBRATION",0,'long')
#kw12 = sb.SRIKeyword("TEMPERATURE",100,'float')
#kw13 = sb.SRIKeyword("DEVICE_IDENTIFIER",1111,'long')
#
#keywords = [kw, kw1, kw2, kw3, kw5, kw6, kw7, kw8, kw9, kw10, kw11, kw12, kw13]
#data = range(30000)
##do not run push data a second time. An invalid time will be sent as a second push will push a time tag of twsec = 0 tfsec = 0
##resulting in a error and test failure
#inputS.push(data, False, "First", 2500000.0,False,keywords,False)
#time.sleep(2)
#
#received_data = outputS.getData()
#passed = True
#if len(received_data) == 0 :
#    passed = False
#for i in range(len(received_data)):
#    if received_data[i] != data[i]:
#       passed = False
#
#f.write('\n********** Stopping Components **********\n\n')
#sb.stop()
#f.write('Components stopped\n')
#
#if passed:
#    print "\nUnit Test 1 ....................",u'\u2714'
#    f.write("\nUnit Test 1 .................... "+u'\u2714'.encode('utf8'))
#else:
#    print "\nUnit Test 1 ....................",u'\u2718'
#    f.write("\nUnit Test 1 .................... "+u'\u2718'.encode('utf8'))
#f.write('\n\n')
#   
#
#
#
##Differences from unit test 1
## 1) sink.VITA49Encapsulation.useVRLFrames = True
#f.write('*********************************\n')
#f.write('********** Unit Test 2 **********\n')
#f.write( '*********************************\n\n')
#
#f.write('********** Creating components and connections **********\n')
#sink = sb.Component('SinkVITA49',execparams={"DEBUG_LEVEL":0})
#source = sb.Component('SourceVITA49',execparams={"DEBUG_LEVEL":0})
##push data into the sink NIC
#inputS=sb.DataSource();
#outputS=sb.DataSink();
#
#sink.connect(source, providesPortName='dataVITA49_in')
#inputS.connect(sink,providesPortName='dataShort_in')
#source.connect(outputS,providesPortName='shortIn')
#
#f.write('***** Source Configuration *****\n')
#source.interface = "em1"
#f.write('source.interface = "em1"\n')
#source.attachment_override.enabled = False
#f.write( 'source.attachment_override.enabled = False\n')
#source.advanced_configuration.corba_transfersize = 2928
#f.write( 'source.advanced_configuration.corba_transfersize = 2928\n')
#source.advanced_configuration.bufferSize = 2048
#f.write( 'source.advanced_configuration.bufferSize = 2048\n')
#source.advanced_configuration.poll_in_time = 0
#f.write( 'source.advanced_configuration.poll_in_time = 0\n\n')
#
#f.write('***** Sink Configuration *****\n')
#sink.network_settings.interface = "em1"
#f.write('sink.network_settings.interface = "em1"\n')
#sink.network_settings.use_udp_protocol = True
#f.write('sink.network_settings.use_udp_protocol = True\n')
#sink.network_settings.vlan = 0
#f.write('sink.network_settings.vlan = 0\n')
#sink.network_settings.port = 29465
#f.write('sink.network_settings.port = 29465\n')
#sink.network_settings.address = ip_address
#f.write('sink.network_settings.address = ip_address\n')
#sink.network_settings.enable = True
#f.write('sink.network_settings.enable = True\n')
##sink.VITA49Encapsulation.useVRLFrames = True
##f.write('sink.VITA49Encapsulation.useVRLFrames = True\n')
#sink.numberOfBuffers = 200
#f.write('sink.numberOfBuffers = 200\n')
#sink.advanced_configuration.endian_representation = 2
#f.write('sink.advanced_configuration.endian_representation = 2\n')
#
#output_result = sb.FileSink('results.tmp')
#source.connect(output_result,providesPortName='shortIn')
#
#f.write('\n********** Starting Components **********\n\n') 
#sb.start()
#f.write('Components started\n')
#
#input_file = sb.FileSource('golden.tmp', dataFormat='short')
#
#kw = sb.SRIKeyword("COL_BW",20000000.0,'double')
#kw1 = sb.SRIKeyword("COL_IF_FREQUENCY_OFFSET",70000000.0,'double')
#kw2 = sb.SRIKeyword("COL_RF",155500000.0,'double')
#kw3 = sb.SRIKeyword("COL_RF_OFFSET",20.0,'double')
#kw5 = sb.SRIKeyword("COL_REFERENCE_LEVEL",10.2969,'float')
#kw6 = sb.SRIKeyword("COL_GAIN",10.0,'float')
#kw7 = sb.SRIKeyword("DATA_GAIN",0.0,'float')
#kw8 = sb.SRIKeyword("ATTENUATION_SUM",10.0,'float')
#kw9 = sb.SRIKeyword("OVER_RANGE_SUM",0,'long')
#kw10 = sb.SRIKeyword("TIMESTAMP_ADJUSTMENT",0,'long')
#kw11 = sb.SRIKeyword("TIMESTAMP_CALIBRATION",0,'long')
#kw12 = sb.SRIKeyword("TEMPERATURE",100,'float')
#kw13 = sb.SRIKeyword("DEVICE_IDENTIFIER",1111,'long')
#
#keywords = [kw, kw1, kw2, kw3, kw5, kw6, kw7, kw8, kw9, kw10, kw11, kw12, kw13]
#data = range(30000)
##do not run push data a second time. An invalid time will be sent as a second push will push a time tag of twsec = 0 tfsec = 0
##resulting in a error and test failure
#inputS.push(data, False, "First", 2500000.0,False,keywords,False)
#time.sleep(2)
#
#received_data = outputS.getData()
#passed = True
#if len(received_data) == 0 :
#    passed = False
#for i in range(len(received_data)):
#    if received_data[i] != data[i]:
#       passed = False
#
#f.write('\n********** Stopping Components **********\n\n')
#sb.stop()
#f.write('Components stopped\n')
#
#if passed:
#    print "\nUnit Test 2 ....................",u'\u2714'
#    f.write("\nUnit Test 2 .................... "+u'\u2714'.encode('utf8'))
#else:
#    print "\nUnit Test 2 ....................",u'\u2718'
#    f.write("\nUnit Test 2 .................... "+u'\u2718'.encode('utf8'))
#f.write('\n\n')
#
#
#
#
##Differences from unit test 2
## 1) sink.VITA49Encapsulation.useCRC = True
#f.write('*********************************\n')
#f.write('********** Unit Test 3 **********\n')
#f.write( '*********************************\n\n')
#
#f.write('********** Creating components and connections **********\n')
#sink = sb.Component('SinkVITA49',execparams={"DEBUG_LEVEL":0})
#source = sb.Component('SourceVITA49',execparams={"DEBUG_LEVEL":0})
##push data into the sink NIC
#inputS=sb.DataSource();
#outputS=sb.DataSink();
#
#sink.connect(source, providesPortName='dataVITA49_in')
#inputS.connect(sink,providesPortName='dataShort_in')
#source.connect(outputS,providesPortName='shortIn')
#
#f.write('***** Source Configuration *****\n')
#source.interface = "em1"
#f.write('source.interface = "em1"\n')
#source.attachment_override.enabled = False
#f.write( 'source.attachment_override.enabled = False\n')
#source.advanced_configuration.corba_transfersize = 2928
#f.write( 'source.advanced_configuration.corba_transfersize = 2928\n')
#source.advanced_configuration.buffer_size = 2048
#f.write( 'source.advanced_configuration.bufferSize = 2048\n')
#source.advanced_configuration.poll_in_time = 10
#f.write( 'source.advanced_configuration.poll_in_time = 0\n\n')
#
#f.write('***** Sink Configuration *****\n')
#sink.network_settings.interface = "em1"
#f.write('sink.network_settings.interface = "em1"\n')
#sink.network_settings.use_udp_protocol = True
#f.write('sink.network_settings.use_udp_protocol = True\n')
#sink.network_settings.vlan = 0
#f.write('sink.network_settings.vlan = 0\n')
#sink.network_settings.port = 29465
#f.write('sink.network_settings.port = 29465\n')
#sink.network_settings.address = ip_address
#f.write('sink.network_settings.address = ip_address\n')
#sink.network_settings.enable = True
#f.write('sink.network_settings.enable = True\n')
#sink.VITA49Encapsulation.useVRLFrames = True
#f.write('sink.VITA49Encapsulation.useVRLFrames = True\n')
#sink.VITA49Encapsulation.useCRC = True
#f.write('sink.VITA49Encapsulation.useCRC = True\n')
#sink.numberOfBuffers = 200
#f.write('sink.numberOfBuffers = 200\n')
#sink.advanced_configuration.endian_representation = 2
#f.write('sink.advanced_configuration.endian_representation = 2\n')
#
#output_result = sb.FileSink('results.tmp')
#source.connect(output_result,providesPortName='shortIn')
#
#f.write('\n********** Starting Components **********\n\n') 
#sb.start()
#f.write('Components started\n')
#
#input_file = sb.FileSource('golden.tmp', dataFormat='short')
#
#kw = sb.SRIKeyword("COL_BW",20000000.0,'double')
#kw1 = sb.SRIKeyword("COL_IF_FREQUENCY_OFFSET",70000000.0,'double')
#kw2 = sb.SRIKeyword("COL_RF",155500000.0,'double')
#kw3 = sb.SRIKeyword("COL_RF_OFFSET",20.0,'double')
#kw5 = sb.SRIKeyword("COL_REFERENCE_LEVEL",10.2969,'float')
#kw6 = sb.SRIKeyword("COL_GAIN",10.0,'float')
#kw7 = sb.SRIKeyword("DATA_GAIN",0.0,'float')
#kw8 = sb.SRIKeyword("ATTENUATION_SUM",10.0,'float')
#kw9 = sb.SRIKeyword("OVER_RANGE_SUM",0,'long')
#kw10 = sb.SRIKeyword("TIMESTAMP_ADJUSTMENT",0,'long')
#kw11 = sb.SRIKeyword("TIMESTAMP_CALIBRATION",0,'long')
#kw12 = sb.SRIKeyword("TEMPERATURE",100,'float')
#kw13 = sb.SRIKeyword("DEVICE_IDENTIFIER",1111,'long')
#
#keywords = [kw, kw1, kw2, kw3, kw5, kw6, kw7, kw8, kw9, kw10, kw11, kw12, kw13]
#data = range(30000)
##do not run push data a second time. An invalid time will be sent as a second push will push a time tag of twsec = 0 tfsec = 0
##resulting in a error and test failure
#inputS.push(data, False, "First", 2500000.0,False,keywords,False)
#time.sleep(2)
#
#received_data = outputS.getData()
#passed = True
#if len(received_data) == 0 :
#    passed = False
#for i in range(len(received_data)):
#    if received_data[i] != data[i]:
#       passed = False
#
#f.write('\n********** Stopping Components **********\n\n')
#sb.stop()
#f.write('Components stopped\n')
#
#if passed:
#    print "\nUnit Test 3 ....................",u'\u2714'
#    f.write("\nUnit Test 3 .................... "+u'\u2714'.encode('utf8'))
#else:
#    print "\nUnit Test 3 ....................",u'\u2718'
#    f.write("\nUnit Test 3 .................... "+u'\u2718'.encode('utf8'))
#f.write('\n\n')
#
##Differences from unit test 4
## 1) sink.VITA49Encapsulation.use_udp_protocol = False
#f.write('*********************************\n')
#f.write('********** Unit Test 4 **********\n')
#f.write( '*********************************\n\n')
#
#f.write('********** Creating components and connections **********\n')
#sink = sb.Component('SinkVITA49',execparams={"DEBUG_LEVEL":0})
#source = sb.Component('SourceVITA49',execparams={"DEBUG_LEVEL":0})
##push data into the sink NIC
#inputS=sb.DataSource();
#outputS=sb.DataSink();
#
#sink.connect(source, providesPortName='dataVITA49_in')
#inputS.connect(sink,providesPortName='dataShort_in')
#source.connect(outputS,providesPortName='shortIn')
#
#f.write('***** Source Configuration *****\n')
#source.interface = "em1"
#f.write('source.interface = "em1"\n')
#source.attachment_override.enabled = False
#f.write( 'source.attachment_override.enabled = False\n')
#source.advanced_configuration.corba_transfersize = 2928
#f.write( 'source.advanced_configuration.corba_transfersize = 2928\n')
#source.advanced_configuration.bufferSize = 2048
#f.write( 'source.advanced_configuration.bufferSize = 2048\n')
#source.advanced_configuration.poll_in_time = 0
#f.write( 'source.advanced_configuration.poll_in_time = 0\n\n')
#
#f.write('***** Sink Configuration *****\n')
#sink.network_settings.interface = "em1"
#f.write('sink.network_settings.interface = "em1"\n')
#sink.network_settings.use_udp_protocol = False
#f.write('sink.network_settings.use_udp_protocol = False\n')
#sink.network_settings.vlan = 0
#f.write('sink.network_settings.vlan = 0\n')
#sink.network_settings.port = 29465
#f.write('sink.network_settings.port = 29465\n')
#sink.network_settings.address = ip_address
#f.write('sink.network_settings.address = ip_address\n')
#sink.network_settings.enable = True
#f.write('sink.network_settings.enable = True\n')
#sink.VITA49Encapsulation.useVRLFrames = True
#f.write('sink.VITA49Encapsulation.useVRLFrames = True\n')
#sink.VITA49Encapsulation.useCRC = True
#f.write('sink.VITA49Encapsulation.useCRC = True\n')
#sink.numberOfBuffers = 200
#f.write('sink.numberOfBuffers = 200\n')
#sink.advanced_configuration.endian_representation = 2
#f.write('sink.advanced_configuration.endian_representation = 2\n')
#
#output_result = sb.FileSink('results.tmp')
#source.connect(output_result,providesPortName='shortIn')
#
#f.write('\n********** Starting Components **********\n\n') 
#sb.start()
#f.write('Components started\n')
#
#input_file = sb.FileSource('golden.tmp', dataFormat='short')
#
#kw = sb.SRIKeyword("COL_BW",20000000.0,'double')
#kw1 = sb.SRIKeyword("COL_IF_FREQUENCY_OFFSET",70000000.0,'double')
#kw2 = sb.SRIKeyword("COL_RF",155500000.0,'double')
#kw3 = sb.SRIKeyword("COL_RF_OFFSET",20.0,'double')
#kw5 = sb.SRIKeyword("COL_REFERENCE_LEVEL",10.2969,'float')
#kw6 = sb.SRIKeyword("COL_GAIN",10.0,'float')
#kw7 = sb.SRIKeyword("DATA_GAIN",0.0,'float')
#kw8 = sb.SRIKeyword("ATTENUATION_SUM",10.0,'float')
#kw9 = sb.SRIKeyword("OVER_RANGE_SUM",0,'long')
#kw10 = sb.SRIKeyword("TIMESTAMP_ADJUSTMENT",0,'long')
#kw11 = sb.SRIKeyword("TIMESTAMP_CALIBRATION",0,'long')
#kw12 = sb.SRIKeyword("TEMPERATURE",100,'float')
#kw13 = sb.SRIKeyword("DEVICE_IDENTIFIER",1111,'long')
#
#keywords = [kw, kw1, kw2, kw3, kw5, kw6, kw7, kw8, kw9, kw10, kw11, kw12, kw13]
#data = range(30000)
##do not run push data a second time. An invalid time will be sent as a second push will push a time tag of twsec = 0 tfsec = 0
##resulting in a error and test failure
#inputS.push(data, False, "First", 2500000.0,False,keywords,False)
#time.sleep(2)
#
#received_data = outputS.getData()
#passed = True
#if len(received_data) == 0 :
#    passed = False
#for i in range(len(received_data)):
#    if received_data[i] != data[i]:
#       passed = False
#
#f.write('\n********** Stopping Components **********\n\n')
#sb.stop()
#f.write('Components stopped\n')
#
#if passed:
#    print "\nUnit Test 4 ....................",u'\u2714'
#    f.write("\nUnit Test 4 .................... "+u'\u2714'.encode('utf8'))
#else:
#    print "\nUnit Test 4 ....................",u'\u2718'
#    f.write("\nUnit Test 4 .................... "+u'\u2718'.encode('utf8'))
#f.write('\n\n')


f.write('*********************************\n')
f.write('********** Unit Test 5 **********\n')
f.write( '*********************************\n\n')

# This unit test relies on rhSourceVITA49 component for end-to-end testing
if "rh.SourceVITA49" not in sb.catalog():
    print "ERROR - cannot run tests without rh.SourceVITA49. Install in $SDRROOT and try again."
    sys.exit(1)

f.write('********** Creating components and connections **********\n')
sink = sb.launch('../SinkVITA49.spd.xml',execparams={"DEBUG_LEVEL":0})
source = sb.launch('rh.SourceVITA49',execparams={"DEBUG_LEVEL":0})
#push data into the sink NIC
inputS=sb.DataSource();
outputS=sb.DataSink();

sink.connect(source, providesPortName='dataVITA49_in')
inputS.connect(sink,providesPortName='dataShort_in')
source.connect(outputS,providesPortName='shortIn')

f.write( '***** Source Configuration *****\n')
source.interface = ethernet_device
f.write( 'source.interface = "em1"\n')
source.attachment_override.enabled = False
f.write( 'source.attachment_override.enabled = True\n')
source.attachment_override.ip_address = ip_address
f.write( 'source.attachment_override.ip_address = ip_address\n')
source.attachment_override.port = network_port
f.write( 'source.attachment_override.port = 29465')
source.attachment_override.use_udp_protocol = True
f.write( 'source.attachment_override.use_udp_protocol = True\n')
source.advanced_configuration.corba_transfersize = 2928
f.write( 'source.advanced_configuration.corba_transfersize = 2928\n')
source.advanced_configuration.bufferSize = 2048
f.write( 'source.advanced_configuration.bufferSize = 2048\n')
source.advanced_configuration.poll_in_time = 10
f.write( 'source.advanced_configuration.poll_in_time = 100\n')
source.advanced_configuration.vita49_packet_size = 1500
f.write( 'source.advanced_configuration.vita49_packet_size = 1500\n\n')

f.write('***** Sink Configuration *****\n')
sink.network_settings.interface = ethernet_device
f.write('sink.network_settings.interface = "em1"\n')
sink.network_settings.use_udp_protocol = True
f.write('sink.network_settings.use_udp_protocol = True\n')
sink.network_settings.vlan = 0
f.write('sink.network_settings.vlan = 0\n')
sink.network_settings.port = network_port
f.write('sink.network_settings.port = 29465\n')
sink.network_settings.ip_address = ip_address
f.write('sink.network_settings.address = ip_address\n')
sink.network_settings.enable = True
f.write('sink.network_settings.enable = True\n')
sink.VITA49Encapsulation.use_vrl_frames = True
f.write('sink.VITA49Encapsulation.useVRLFrames = True\n')
sink.VITA49Encapsulation.use_crc = False
f.write('sink.VITA49Encapsulation.useCRC = True\n')
sink.numberOfBuffers = 200
f.write('sink.numberOfBuffers = 200\n')
sink.advanced_configuration.endian_representation = 2
f.write('sink.advanced_configuration.endian_representation = 2\n')
sink.advanced_configuration.max_payload_size = 1428
f.write('sink.advanced_configuration.max_payload_size = 1500\n\n')
sink.VITA49IFContextPacket.class_identifier = "DEFAULT"
sink.VITA49IFContextPacket.device_identifier = "FF-FF-FA:1333"
output_result = sb.FileSink('results.tmp')
source.connect(output_result,providesPortName='shortIn')
#sink.api() 
#time.sleep(100)
f.write('\n********** Starting Components **********\n') 
sb.start()
f.write('Components started\n')

input_file = sb.FileSource('golden.tmp', dataFormat='short')

kw = sb.SRIKeyword("COL_BW",20000000.0,'double')
kw1 = sb.SRIKeyword("COL_IF_FREQUENCY_OFFSET",70000000.0,'double')
kw2 = sb.SRIKeyword("COL_RF",155500000.0,'double')
kw3 = sb.SRIKeyword("COL_RF_OFFSET",20.0,'double')
kw5 = sb.SRIKeyword("COL_REFERENCE_LEVEL",10.2969,'float')
kw6 = sb.SRIKeyword("COL_GAIN",10.0,'float')
kw7 = sb.SRIKeyword("DATA_GAIN",0.0,'float')
kw8 = sb.SRIKeyword("ATTENUATION_SUM",10.0,'float')
kw9 = sb.SRIKeyword("OVER_RANGE_SUM",0,'long')
kw10 = sb.SRIKeyword("TIMESTAMP_ADJUSTMENT",2,'long')
kw11 = sb.SRIKeyword("TIMESTAMP_CALIBRATION",100,'long')
kw12 = sb.SRIKeyword("TEMPERATURE",100,'float')
kw13 = sb.SRIKeyword("DEVICE_IDENTIFIER",1111,'long')

subkw  = CF.DataType('GEOLOCATION_GPS::TIME_SECONDS', CORBA.Any(CORBA._tc_double, 1111))
subkw1 = CF.DataType('GEOLOCATION_GPS::TIME_FRACTIONAL',CORBA.Any(CORBA._tc_double, .00002))
subkw2 = CF.DataType('GEOLOCATION_GPS::MANUFACTURER_ID',CORBA.Any(CORBA._tc_long, 123546))
subkw3 = CF.DataType('GEOLOCATION_GPS::LATITUDE',CORBA.Any(CORBA._tc_double, 10.0))
subkw4 = CF.DataType('GEOLOCATION_GPS::LONGITUDE',CORBA.Any(CORBA._tc_double, 25.0))
subkw5 = CF.DataType('GEOLOCATION_GPS::ALTITUDE',CORBA.Any(CORBA._tc_double, 32.0))
subkw6 = CF.DataType('GEOLOCATION_GPS::GROUND_SPEED',CORBA.Any(CORBA._tc_double, 26.0))
subkw7 = CF.DataType('GEOLOCATION_GPS::HEADING_ANGLE',CORBA.Any(CORBA._tc_double, 36.0))
subkw8 = CF.DataType('GEOLOCATION_GPS::TRACK_ANGLE',CORBA.Any(CORBA._tc_double, 15.0))
subkw9 = CF.DataType('GEOLOCATION_GPS::MAGNETIC_VARIATION',CORBA.Any(CORBA._tc_double, 16.0))

list = [subkw, subkw1, subkw2, subkw3, subkw4, subkw5, subkw6, subkw7, subkw8, subkw9]
value = sb.SRIKeyword('GEOLOCATION_GPS',properties.props_to_any(list), 'IDL:CF/Properties:1.0')

subkw  = CF.DataType('GEOLOCATION_INS::TIME_SECONDS', CORBA.Any(CORBA._tc_double, 1111))
subkw1 = CF.DataType('GEOLOCATION_INS::TIME_FRACTIONAL',CORBA.Any(CORBA._tc_double, .00002))
subkw2 = CF.DataType('GEOLOCATION_INS::MANUFACTURER_ID',CORBA.Any(CORBA._tc_long, 123546))
subkw3 = CF.DataType('GEOLOCATION_INS::LATITUDE',CORBA.Any(CORBA._tc_double, 10.0))
subkw4 = CF.DataType('GEOLOCATION_INS::LONGITUDE',CORBA.Any(CORBA._tc_double, 25.0))
subkw5 = CF.DataType('GEOLOCATION_INS::ALTITUDE',CORBA.Any(CORBA._tc_double, 32.0))
subkw6 = CF.DataType('GEOLOCATION_INS::GROUND_SPEED',CORBA.Any(CORBA._tc_double, 26.0))
subkw7 = CF.DataType('GEOLOCATION_INS::HEADING_ANGLE',CORBA.Any(CORBA._tc_double, 36.0))
subkw8 = CF.DataType('GEOLOCATION_INS::TRACK_ANGLE',CORBA.Any(CORBA._tc_double, 15.0))
subkw9 = CF.DataType('GEOLOCATION_INS::MAGNETIC_VARIATION',CORBA.Any(CORBA._tc_double, 16.0))

list = [subkw, subkw1, subkw2, subkw3, subkw4, subkw5, subkw6, subkw7, subkw8, subkw9]
value1 = sb.SRIKeyword('GEOLOCATION_INS',properties.props_to_any(list), 'IDL:CF/Properties:1.0')

subkw  = CF.DataType('EPHEMERIS_ECEF::TIME_SECONDS', CORBA.Any(CORBA._tc_double, 1111))
subkw1 = CF.DataType('EPHEMERIS_ECEF::TIME_FRACTIONAL_SECONDS',CORBA.Any(CORBA._tc_double, .00002))
subkw2 = CF.DataType('EPHEMERIS_ECEF::POSITION_X',CORBA.Any(CORBA._tc_double, 123546))
subkw3 = CF.DataType('EPHEMERIS_ECEF::POSITION_Y',CORBA.Any(CORBA._tc_double, 10.0))
subkw4 = CF.DataType('EPHEMERIS_ECEF::POSITION_Z',CORBA.Any(CORBA._tc_double, 25.0))
subkw5 = CF.DataType('EPHEMERIS_ECEF::ATTITUDE_ALPHA',CORBA.Any(CORBA._tc_double, 32.0))
subkw6 = CF.DataType('EPHEMERIS_ECEF::ATTITUDE_BETA',CORBA.Any(CORBA._tc_double, 26.0))
subkw7 = CF.DataType('EPHEMERIS_ECEF::ATTITUDE_PHI',CORBA.Any(CORBA._tc_double, 36.0))
subkw8 = CF.DataType('EPHEMERIS_ECEF::VELOCITY_X',CORBA.Any(CORBA._tc_double, 15.0))
subkw9 = CF.DataType('EPHEMERIS_ECEF::VELOCITY_Y',CORBA.Any(CORBA._tc_double, 16.0))
subkw10 = CF.DataType('EPHEMERIS_ECEF::VELOCITY_Z',CORBA.Any(CORBA._tc_double, 14.0))
subkw11 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_VELOCITY_ALPHA',CORBA.Any(CORBA._tc_double, 16.0))
subkw12 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_VELOCITY_BETA',CORBA.Any(CORBA._tc_double, 13456.0))
subkw13 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_VELOCITY_PHI',CORBA.Any(CORBA._tc_double, 1324.0))
subkw14 = CF.DataType('EPHEMERIS_ECEF::ACCELERATION_X',CORBA.Any(CORBA._tc_double, 43.0))
subkw15 = CF.DataType('EPHEMERIS_ECEF::ACCELERATION_Y',CORBA.Any(CORBA._tc_double, 12.0))
subkw16 = CF.DataType('EPHEMERIS_ECEF::ACCELERATION_Z',CORBA.Any(CORBA._tc_double, 14.0))
subkw17 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_ACCELERATION_ALPHA',CORBA.Any(CORBA._tc_double, 174.0))
subkw18 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_ACCELERATION_BETA',CORBA.Any(CORBA._tc_double, 145.0))
subkw19 = CF.DataType('EPHEMERIS_ECEF::ROTATIONAL_ACCELERATION_PHI',CORBA.Any(CORBA._tc_double, 13.0))

list = [subkw, subkw1, subkw2, subkw3, subkw4, subkw5, subkw6, subkw7, subkw8, subkw9, subkw10, subkw11, subkw12, subkw13, subkw14, subkw15, subkw16, subkw17, subkw18, subkw19]
value2 = sb.SRIKeyword('EPHEMERIS_ECEF',properties.props_to_any(list), 'IDL:CF/Properties:1.0')


subkw  = CF.DataType('EPHEMERIS_RELATIVE::TIME_SECONDS', CORBA.Any(CORBA._tc_double, 32326))
subkw1 = CF.DataType('EPHEMERIS_RELATIVE::TIME_FRACTIONAL_SECONDS',CORBA.Any(CORBA._tc_double, .00004))
subkw2 = CF.DataType('EPHEMERIS_RELATIVE::POSITION_X',CORBA.Any(CORBA._tc_double, 654321))
subkw3 = CF.DataType('EPHEMERIS_RELATIVE::POSITION_Y',CORBA.Any(CORBA._tc_double, 11.0))
subkw4 = CF.DataType('EPHEMERIS_RELATIVE::POSITION_Z',CORBA.Any(CORBA._tc_double, 24.0))
subkw5 = CF.DataType('EPHEMERIS_RELATIVE::ATTITUDE_ALPHA',CORBA.Any(CORBA._tc_double, 25.0))
subkw6 = CF.DataType('EPHEMERIS_RELATIVE::ATTITUDE_BETA',CORBA.Any(CORBA._tc_double, 39.0))
subkw7 = CF.DataType('EPHEMERIS_RELATIVE::ATTITUDE_PHI',CORBA.Any(CORBA._tc_double, 89.0))
subkw8 = CF.DataType('EPHEMERIS_RELATIVE::VELOCITY_X',CORBA.Any(CORBA._tc_double, 26.0))
subkw9 = CF.DataType('EPHEMERIS_RELATIVE::VELOCITY_Y',CORBA.Any(CORBA._tc_double, 77.0))
subkw10 = CF.DataType('EPHEMERIS_RELATIVE::VELOCITY_Z',CORBA.Any(CORBA._tc_double, 663.3))
subkw11 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_VELOCITY_ALPHA',CORBA.Any(CORBA._tc_double, 12.3))
subkw12 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_VELOCITY_BETA',CORBA.Any(CORBA._tc_double, 23.6))
subkw13 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_VELOCITY_PHI',CORBA.Any(CORBA._tc_double, 2658.52))
subkw14 = CF.DataType('EPHEMERIS_RELATIVE::ACCELERATION_X',CORBA.Any(CORBA._tc_double, 2562.24))
subkw15 = CF.DataType('EPHEMERIS_RELATIVE::ACCELERATION_Y',CORBA.Any(CORBA._tc_double, 41485.525))
subkw16 = CF.DataType('EPHEMERIS_RELATIVE::ACCELERATION_Z',CORBA.Any(CORBA._tc_double, 26335.52))
subkw17 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_ACCELERATION_ALPHA',CORBA.Any(CORBA._tc_double, 256325.23))
subkw18 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_ACCELERATION_BETA',CORBA.Any(CORBA._tc_double, 78925.5))
subkw19 = CF.DataType('EPHEMERIS_RELATIVE::ROTATIONAL_ACCELERATION_PHI',CORBA.Any(CORBA._tc_double, 21256.54))

list = [subkw, subkw1, subkw2, subkw3, subkw4, subkw5, subkw6, subkw7, subkw8, subkw9, subkw10, subkw11, subkw12, subkw13, subkw14, subkw15, subkw16, subkw17, subkw18, subkw19]
value3 = sb.SRIKeyword('EPHEMERIS_RELATIVE',properties.props_to_any(list), 'IDL:CF/Properties:1.0')


keywords = [kw, kw1, kw2, kw3, kw5, kw6, kw7, kw8, kw9, kw10, kw11, kw12, kw13, value, value1, value2, value3]
data = range(30000)
#do not run push data a second time. An invalid time will be sent as a second push will push a time tag of twsec = 0 tfsec = 0
#resulting in a error and test failure

inputS.push(data, False, "First", 2500000.0,True,keywords,False)
#print inputS._sri
#inputS.push(data, True, "First", 2500000.0,True,keywords,False)

time.sleep(5)
#inputS.push(data, EOS, streamID, sampleRate, complexData, SRIKeywords, loop)
received_sri = outputS.sri()
received_data = outputS.getData()

#print received_sri
#print len(received_data)
passed = True
if len(received_data) == 0 :
    passed = False
    print "NO DATA SENT"
    f.write('\n********************* NO DATA SENT ***************')
for i in range(len(received_data)):
    if received_data[i] != data[i]:
       passed = False

f.write('\n********** Stopping Components **********\n')
#sb.stop()

#print 'Source stopped'
source.stop()
sink.stop()
#print 'Sink stopped'
f.write('Components stopped\n')

if passed:
    print "\nUnit Test 5 ....................",u'\u2714'
    f.write("\nUnit Test 5 .................... "+u'\u2714'.encode('utf8'))
else:
    print "\nUnit Test 5 ....................",u'\u2716'
    f.write("\nUnit Test 5 .................... "+u'\u2718'.encode('utf8'))
f.write('\n\n')

f.close()

