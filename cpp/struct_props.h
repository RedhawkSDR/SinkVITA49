/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK SinkVITA49.
 *
 * REDHAWK SinkVITA49 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK SinkVITA49 is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>

struct network_settings_struct {
    network_settings_struct ()
    {
        ip_address = "127.0.0.1";
        port = 12344;
        vlan = 0;
        use_udp_protocol = true;
        interface = "eth0";
    };

    static std::string getId() {
        return std::string("network_settings");
    };

    std::string ip_address;
    CORBA::Long port;
    unsigned short vlan;
    bool use_udp_protocol;
    std::string interface;
};

inline bool operator>>= (const CORBA::Any& a, network_settings_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("network_settings::ip_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_address)) return false;
        }
        else if (!strcmp("network_settings::port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
        else if (!strcmp("network_settings::vlan", props[idx].id)) {
            if (!(props[idx].value >>= s.vlan)) return false;
        }
        else if (!strcmp("network_settings::use_udp_protocol", props[idx].id)) {
            if (!(props[idx].value >>= s.use_udp_protocol)) return false;
        }
        else if (!strcmp("network_settings::interface", props[idx].id)) {
            if (!(props[idx].value >>= s.interface)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const network_settings_struct& s) {
    CF::Properties props;
    props.length(5);
    props[0].id = CORBA::string_dup("network_settings::ip_address");
    props[0].value <<= s.ip_address;
    props[1].id = CORBA::string_dup("network_settings::port");
    props[1].value <<= s.port;
    props[2].id = CORBA::string_dup("network_settings::vlan");
    props[2].value <<= s.vlan;
    props[3].id = CORBA::string_dup("network_settings::use_udp_protocol");
    props[3].value <<= s.use_udp_protocol;
    props[4].id = CORBA::string_dup("network_settings::interface");
    props[4].value <<= s.interface;
    a <<= props;
};

inline bool operator== (const network_settings_struct& s1, const network_settings_struct& s2) {
    if (s1.ip_address!=s2.ip_address)
        return false;
    if (s1.port!=s2.port)
        return false;
    if (s1.vlan!=s2.vlan)
        return false;
    if (s1.use_udp_protocol!=s2.use_udp_protocol)
        return false;
    if (s1.interface!=s2.interface)
        return false;
    return true;
};

inline bool operator!= (const network_settings_struct& s1, const network_settings_struct& s2) {
    return !(s1==s2);
};

struct VITA49Encapsulation_struct {
    VITA49Encapsulation_struct ()
    {
        enable_crc = false;
        enable_vrl_frames = true;
    };

    static std::string getId() {
        return std::string("VITA49Encapsulation");
    };

    bool enable_crc;
    bool enable_vrl_frames;
};

inline bool operator>>= (const CORBA::Any& a, VITA49Encapsulation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("VITA49Encapsulation::enable_crc", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_crc)) return false;
        }
        else if (!strcmp("VITA49Encapsulation::enable_vrl_frames", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_vrl_frames)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const VITA49Encapsulation_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("VITA49Encapsulation::enable_crc");
    props[0].value <<= s.enable_crc;
    props[1].id = CORBA::string_dup("VITA49Encapsulation::enable_vrl_frames");
    props[1].value <<= s.enable_vrl_frames;
    a <<= props;
};

inline bool operator== (const VITA49Encapsulation_struct& s1, const VITA49Encapsulation_struct& s2) {
    if (s1.enable_crc!=s2.enable_crc)
        return false;
    if (s1.enable_vrl_frames!=s2.enable_vrl_frames)
        return false;
    return true;
};

inline bool operator!= (const VITA49Encapsulation_struct& s1, const VITA49Encapsulation_struct& s2) {
    return !(s1==s2);
};

struct VITA49IFDataPacket_struct {
    VITA49IFDataPacket_struct ()
    {
        enable = true;
        enable_stream_identifier = true;
        enable_class_identifier = true;
        embed_time_stamp = true;
        enable_trailer = true;
    };

    static std::string getId() {
        return std::string("VITA49IFDataPacket");
    };

    bool enable;
    bool enable_stream_identifier;
    bool enable_class_identifier;
    bool embed_time_stamp;
    bool enable_trailer;
};

inline bool operator>>= (const CORBA::Any& a, VITA49IFDataPacket_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("VITA49IFDataPacket::enable", props[idx].id)) {
            if (!(props[idx].value >>= s.enable)) return false;
        }
        else if (!strcmp("VITA49IFDataPacket::enable_stream_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_stream_identifier)) return false;
        }
        else if (!strcmp("VITA49IFDataPacket::enable_class_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_class_identifier)) return false;
        }
        else if (!strcmp("VITA49IFDataPacket::embed_time_stamp", props[idx].id)) {
            if (!(props[idx].value >>= s.embed_time_stamp)) return false;
        }
        else if (!strcmp("VITA49IFDataPacket::enable_trailer", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_trailer)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const VITA49IFDataPacket_struct& s) {
    CF::Properties props;
    props.length(5);
    props[0].id = CORBA::string_dup("VITA49IFDataPacket::enable");
    props[0].value <<= s.enable;
    props[1].id = CORBA::string_dup("VITA49IFDataPacket::enable_stream_identifier");
    props[1].value <<= s.enable_stream_identifier;
    props[2].id = CORBA::string_dup("VITA49IFDataPacket::enable_class_identifier");
    props[2].value <<= s.enable_class_identifier;
    props[3].id = CORBA::string_dup("VITA49IFDataPacket::embed_time_stamp");
    props[3].value <<= s.embed_time_stamp;
    props[4].id = CORBA::string_dup("VITA49IFDataPacket::enable_trailer");
    props[4].value <<= s.enable_trailer;
    a <<= props;
};

inline bool operator== (const VITA49IFDataPacket_struct& s1, const VITA49IFDataPacket_struct& s2) {
    if (s1.enable!=s2.enable)
        return false;
    if (s1.enable_stream_identifier!=s2.enable_stream_identifier)
        return false;
    if (s1.enable_class_identifier!=s2.enable_class_identifier)
        return false;
    if (s1.embed_time_stamp!=s2.embed_time_stamp)
        return false;
    if (s1.enable_trailer!=s2.enable_trailer)
        return false;
    return true;
};

inline bool operator!= (const VITA49IFDataPacket_struct& s1, const VITA49IFDataPacket_struct& s2) {
    return !(s1==s2);
};

struct VITA49IFContextPacket_struct {
    VITA49IFContextPacket_struct ()
    {
        enable = true;
        enable_stream_identifier = true;
        stream_identifier_offset = 0;
        enable_class_identifier = true;
        class_identifier = "DEFAULT";
        embed_time_stamp = true;
        enable_device_identifier = true;
        device_identifier = "FF-FF-FA:1301";
        enable_trailer = true;
    };

    static std::string getId() {
        return std::string("VITA49IFContextPacket");
    };

    bool enable;
    bool enable_stream_identifier;
    CORBA::Long stream_identifier_offset;
    bool enable_class_identifier;
    std::string class_identifier;
    bool embed_time_stamp;
    bool enable_device_identifier;
    std::string device_identifier;
    bool enable_trailer;
};

inline bool operator>>= (const CORBA::Any& a, VITA49IFContextPacket_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("VITA49IFContextPacket::enable", props[idx].id)) {
            if (!(props[idx].value >>= s.enable)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::enable_stream_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_stream_identifier)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::stream_identifier_offset", props[idx].id)) {
            if (!(props[idx].value >>= s.stream_identifier_offset)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::enable_class_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_class_identifier)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::class_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.class_identifier)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::embed_time_stamp", props[idx].id)) {
            if (!(props[idx].value >>= s.embed_time_stamp)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::enable_device_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_device_identifier)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::device_identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.device_identifier)) return false;
        }
        else if (!strcmp("VITA49IFContextPacket::use_trailer", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_trailer)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const VITA49IFContextPacket_struct& s) {
    CF::Properties props;
    props.length(9);
    props[0].id = CORBA::string_dup("VITA49IFContextPacket::enable");
    props[0].value <<= s.enable;
    props[1].id = CORBA::string_dup("VITA49IFContextPacket::enable_stream_identifier");
    props[1].value <<= s.enable_stream_identifier;
    props[2].id = CORBA::string_dup("VITA49IFContextPacket::stream_identifier_offset");
    props[2].value <<= s.stream_identifier_offset;
    props[3].id = CORBA::string_dup("VITA49IFContextPacket::enable_class_identifier");
    props[3].value <<= s.enable_class_identifier;
    props[4].id = CORBA::string_dup("VITA49IFContextPacket::class_identifier");
    props[4].value <<= s.class_identifier;
    props[5].id = CORBA::string_dup("VITA49IFContextPacket::embed_time_stamp");
    props[5].value <<= s.embed_time_stamp;
    props[6].id = CORBA::string_dup("VITA49IFContextPacket::enable_device_identifier");
    props[6].value <<= s.enable_device_identifier;
    props[7].id = CORBA::string_dup("VITA49IFContextPacket::device_identifier");
    props[7].value <<= s.device_identifier;
    props[8].id = CORBA::string_dup("VITA49IFContextPacket::use_trailer");
    props[8].value <<= s.enable_trailer;
    a <<= props;
};

inline bool operator== (const VITA49IFContextPacket_struct& s1, const VITA49IFContextPacket_struct& s2) {
    if (s1.enable!=s2.enable)
        return false;
    if (s1.enable_stream_identifier!=s2.enable_stream_identifier)
        return false;
    if (s1.stream_identifier_offset!=s2.stream_identifier_offset)
        return false;
    if (s1.enable_class_identifier!=s2.enable_class_identifier)
        return false;
    if (s1.class_identifier!=s2.class_identifier)
        return false;
    if (s1.embed_time_stamp!=s2.embed_time_stamp)
        return false;
    if (s1.enable_device_identifier!=s2.enable_device_identifier)
        return false;
    if (s1.device_identifier!=s2.device_identifier)
        return false;
    if (s1.enable_trailer!=s2.enable_trailer)
        return false;
    return true;
};

inline bool operator!= (const VITA49IFContextPacket_struct& s1, const VITA49IFContextPacket_struct& s2) {
    return !(s1==s2);
};

struct advanced_configuration_struct {
    advanced_configuration_struct ()
    {
        force_transmit = false;
        max_payload_size = 1452;
        endian_representation = 0;
        number_of_buffers = 10;
        use_bulkio_sri = false;
        time_between_context_packets = 1;
        number_of_packets_in_burst = 150;
        throttle_time_between_packet_bursts = 100;
    };

    static std::string getId() {
        return std::string("advanced_configuration");
    };

    bool force_transmit;
    CORBA::Long max_payload_size;
    CORBA::Long endian_representation;
    CORBA::ULong number_of_buffers;
    bool use_bulkio_sri;
    CORBA::Long time_between_context_packets;
    CORBA::Long number_of_packets_in_burst;
    CORBA::Long throttle_time_between_packet_bursts;
};

inline bool operator>>= (const CORBA::Any& a, advanced_configuration_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("advanced_configuration::force_transmit", props[idx].id)) {
            if (!(props[idx].value >>= s.force_transmit)) return false;
        }
        else if (!strcmp("advanced_configuration::max_payload_size", props[idx].id)) {
            if (!(props[idx].value >>= s.max_payload_size)) return false;
        }
        else if (!strcmp("advanced_configuration::endian_representation", props[idx].id)) {
            if (!(props[idx].value >>= s.endian_representation)) return false;
        }
        else if (!strcmp("advanced_configuration::number_of_buffers", props[idx].id)) {
            if (!(props[idx].value >>= s.number_of_buffers)) return false;
        }
        else if (!strcmp("advanced_configuration::use_bulkio_sri", props[idx].id)) {
            if (!(props[idx].value >>= s.use_bulkio_sri)) return false;
        }
        else if (!strcmp("advanced_configuration::time_between_context_packets", props[idx].id)) {
            if (!(props[idx].value >>= s.time_between_context_packets)) return false;
        }
        else if (!strcmp("advanced_configuration::number_of_packets_in_burst", props[idx].id)) {
            if (!(props[idx].value >>= s.number_of_packets_in_burst)) return false;
        }
        else if (!strcmp("advanced_configuration::throttle_time_between_packet_bursts", props[idx].id)) {
            if (!(props[idx].value >>= s.throttle_time_between_packet_bursts)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const advanced_configuration_struct& s) {
    CF::Properties props;
    props.length(8);
    props[0].id = CORBA::string_dup("advanced_configuration::force_transmit");
    props[0].value <<= s.force_transmit;
    props[1].id = CORBA::string_dup("advanced_configuration::max_payload_size");
    props[1].value <<= s.max_payload_size;
    props[2].id = CORBA::string_dup("advanced_configuration::endian_representation");
    props[2].value <<= s.endian_representation;
    props[3].id = CORBA::string_dup("advanced_configuration::number_of_buffers");
    props[3].value <<= s.number_of_buffers;
    props[4].id = CORBA::string_dup("advanced_configuration::use_bulkio_sri");
    props[4].value <<= s.use_bulkio_sri;
    props[5].id = CORBA::string_dup("advanced_configuration::time_between_context_packets");
    props[5].value <<= s.time_between_context_packets;
    props[6].id = CORBA::string_dup("advanced_configuration::number_of_packets_in_burst");
    props[6].value <<= s.number_of_packets_in_burst;
    props[7].id = CORBA::string_dup("advanced_configuration::throttle_time_between_packet_bursts");
    props[7].value <<= s.throttle_time_between_packet_bursts;
    a <<= props;
};

inline bool operator== (const advanced_configuration_struct& s1, const advanced_configuration_struct& s2) {
    if (s1.force_transmit!=s2.force_transmit)
        return false;
    if (s1.max_payload_size!=s2.max_payload_size)
        return false;
    if (s1.endian_representation!=s2.endian_representation)
        return false;
    if (s1.number_of_buffers!=s2.number_of_buffers)
        return false;
    if (s1.use_bulkio_sri!=s2.use_bulkio_sri)
        return false;
    if (s1.time_between_context_packets!=s2.time_between_context_packets)
        return false;
    if (s1.number_of_packets_in_burst!=s2.number_of_packets_in_burst)
        return false;
    if (s1.throttle_time_between_packet_bursts!=s2.throttle_time_between_packet_bursts)
        return false;
    return true;
};

inline bool operator!= (const advanced_configuration_struct& s1, const advanced_configuration_struct& s2) {
    return !(s1==s2);
};

struct connection_status_struct {
    connection_status_struct ()
    {
    };

    static std::string getId() {
        return std::string("connection_status");
    };

    CORBA::Long packet_size;
};

inline bool operator>>= (const CORBA::Any& a, connection_status_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("connection_status::packet_size", props[idx].id)) {
            if (!(props[idx].value >>= s.packet_size)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const connection_status_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("connection_status::packet_size");
    props[0].value <<= s.packet_size;
    a <<= props;
};

inline bool operator== (const connection_status_struct& s1, const connection_status_struct& s2) {
    if (s1.packet_size!=s2.packet_size)
        return false;
    return true;
};

inline bool operator!= (const connection_status_struct& s1, const connection_status_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
