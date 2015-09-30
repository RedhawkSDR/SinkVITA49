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

# Full functionality is tested via end-to-end testing using SinkVITA49 and SourceVITA49 in the fulltest_VITA49.py file
class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in rh.SinkVITA49"""


    def setUp(self):
        """Set up the unit test - this is run before every method that starts with test
        """
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        
        self.componentSetup()
        
        
    def tearDown(self):
        """Finish the unit test - this is run after every method that starts with test
        """
        self.comp.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp.releaseObject()
           
        ossie.utils.testing.ScaComponentTestCase.tearDown(self)


    def componentSetup(self):
        """Standard start-up for testing the component
        """
        print ' * Note:'
        print ' * During this unit test, SinkVITA49 will warn about using a built-in table'
        print ' * for leap seconds if no leap seconds file is found at the indicated location.'
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        execparams["DEBUG_LEVEL"] = DEBUG_LEVEL
        self.launch(execparams, initialize=True)
        
        #######################################################################
        # Simulate regular component startup
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp.configure(configureProps)
        
        self.comp.start()
        
        
    def testStartStopRelease(self):
        """testStartStopRelease - Only start, stop, then release the component.
        """
        # This is all accomplished by the setUp and tearDown functions called for each test.
        # So, do nothing.
        pass
    
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        #self.comp.start()
        #self.comp.stop()

        #######################################################################
        # Simulate regular resource shutdown
        #self.comp.releaseObject()


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


    def testBasicDataInput(self):
        """testBasicDataInput
        """
        #######################################################################
        # Configure properties
        self.comp.network_settings.Address = "127.0.0.1"
        self.comp.Interface = "eth0"
        self.comp.network_settings.vlan = 0;
        self.comp.network_settings.Port = 24967
        self.comp.network_settings.useUDP = True
        self.comp.network_settings.MTU = 1500
        self.comp.network_settings.enable = True
        self.comp.numberOfBuffers = 200000

        #######################################################################
        # Push SRI and data to the component
        dh = bulkio_data_helpers.ArraySource(BULKIO__POA.dataShort)
        dh.connectPort(self.comp_obj.getPort("dataShort_in"), "connectionId")

        # Build SRI w/ all expected keywords
        defaultKeywords = []
        defaultKeywords.append(CF.DataType(id="COL_BW",value=CORBA.Any(CORBA.TC_double,20000000)))
        defaultKeywords.append(CF.DataType(id="COL_IF_FREQUENCY_OFFSET",value=CORBA.Any(CORBA.TC_double,70000000)))
        defaultKeywords.append(CF.DataType(id="COL_RF",value=CORBA.Any(CORBA.TC_double,155500000)))
        defaultKeywords.append(CF.DataType(id="COL_RF_OFFSET",value=CORBA.Any(CORBA.TC_double,20)))
        defaultKeywords.append(CF.DataType(id="COL_IF_FREQUENCY_OFFSET",value=CORBA.Any(CORBA.TC_double,30999)))
        defaultKeywords.append(CF.DataType(id="COL_REFERENCE_LEVEL", value=CORBA.Any(CORBA.TC_float,10.2969)))
        defaultKeywords.append(CF.DataType(id="COL_GAIN",value=CORBA.Any(CORBA.TC_float,10)))
        defaultKeywords.append(CF.DataType(id="DATA_GAIN",value=CORBA.Any(CORBA.TC_float,0)))
        defaultKeywords.append(CF.DataType(id="ATTENUATION_SUM",value=CORBA.Any(CORBA.TC_float,10)))
        defaultKeywords.append(CF.DataType(id="OVER_RANGE_SUM",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="TIMESTAMP_ADJUSTMENT",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="TIMESTAMP_CALIBRATION",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="TEMPERATURE",value=CORBA.Any(CORBA.TC_float,100)))
        defaultKeywords.append(CF.DataType(id="DEVICE_IDENTIFIER",value=CORBA.Any(CORBA.TC_long,4368)))
        defaultKeywords.append(CF.DataType(id="CALIBRATED_TIME_STAMP",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="REFERENCE_LOCKED",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="AUTO_GAIN_CONTROL",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="SIGNAL_DETECTION",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="DATA_INVERSION_FLAG",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="OVER_RANGE",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="SAMPLE_LOSS_INDICATOR",value=CORBA.Any(CORBA.TC_boolean,True)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.TIME_SECONDS",value=CORBA.Any(CORBA.TC_long,1029071060)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.TIME_FRACTIONAL_SECONDS",value=CORBA.Any(CORBA.TC_long,-1554051584)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.MANUFACTURER_ID",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.LAT",value=CORBA.Any(CORBA.TC_double,39.1222)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.LON",value=CORBA.Any(CORBA.TC_double,-76.772)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.ALT",value=CORBA.Any(CORBA.TC_double,225)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.GROUND_SPEED",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.HEADING_ANGLE",value=CORBA.Any(CORBA.TC_double,10)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.TRACKING_ANGLE",value=CORBA.Any(CORBA.TC_double,14)))
        defaultKeywords.append(CF.DataType(id="GEO_GPS.MAGNETIC_VARIATION",value=CORBA.Any(CORBA.TC_double,1)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.TIME_SECONDS",value=CORBA.Any(CORBA.TC_long,1029071060)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.TIME_FRACTIONAL_SECONDS",value=CORBA.Any(CORBA.TC_long,-1554051584)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.MANUFACTURER_ID",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.LAT",value=CORBA.Any(CORBA.TC_double,3.12223)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.LON",value=CORBA.Any(CORBA.TC_double,-16.772)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.ALT",value=CORBA.Any(CORBA.TC_double,990)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.GROUND_SPEED",value=CORBA.Any(CORBA.TC_double,1)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.HEADING_ANGLE",value=CORBA.Any(CORBA.TC_double,87)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.TRACKING_ANGLE",value=CORBA.Any(CORBA.TC_double,39)))
        defaultKeywords.append(CF.DataType(id="GEO_INS.MAGNETIC_VARIATION",value=CORBA.Any(CORBA.TC_double,2)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.TIME_SECONDS",value=CORBA.Any(CORBA.TC_long,1029071060)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.TIME_FRACTIONAL_SECONDS",value=CORBA.Any(CORBA.TC_long,-1554051584)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.MANUFACTURER_ID",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.POS_X",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.POS_Y",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.POS_Z",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.ALT_ALPHA",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.ALT_BETA",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.ALT_PHI",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.VEL_X",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.VEL_Y",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_ECEF.VEL_Z",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.TIME_SECONDS",value=CORBA.Any(CORBA.TC_long,1029071060)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.TIME_FRACTIONAL_SECONDS",value=CORBA.Any(CORBA.TC_long,-1554051584)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.MANUFACTURER_ID",value=CORBA.Any(CORBA.TC_long,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.POS_X",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.POS_Y",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.POS_Z",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.ALT_ALPHA",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.ALT_BETA",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.ALT_PHI",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.VEL_X",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.VEL_Y",value=CORBA.Any(CORBA.TC_double,0)))
        defaultKeywords.append(CF.DataType(id="EPH_RELATIVE.VEL_Z",value=CORBA.Any(CORBA.TC_double,0)))

        H = BULKIO.StreamSRI(1, 0.0, 0.0001, 1, 0, 0.0, 0.0, 1,
                    1, "defaultStreamID", False, defaultKeywords)
    
        currentSampleTime = 0
        T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(currentSampleTime), currentSampleTime - int(currentSampleTime))
    
        # Push SRI and data
        dh.pushSRI(H)
        data = range(10000)
        dh.pushPacket(data, T, True, "defaultStreamID")


if __name__ == "__main__":
    ossie.utils.testing.main("../SinkVITA49.spd.xml") # By default tests all implementations
