//
//  Created by Lukas Stahlbock in 2022
//  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
//
#ifndef VSOMEIP_SERVICE_IDS_HPP
#define VSOMEIP_SERVICE_IDS_HPP

#include <vsomeip/vsomeip.hpp>

static vsomeip::service_t service_id_list[] = {0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009,
                                                0x2010, 0x2011, 0x2012, 0x2013, 0x2014, 0x2015, 0x2016, 0x2017, 0x2018, 0x2019,
                                                0x2020, 0x2021, 0x2022, 0x2023, 0x2024, 0x2025, 0x2026, 0x2027, 0x2028, 0x2029,
                                                0x2030, 0x2031, 0x2032, 0x2033, 0x2034, 0x2035, 0x2036, 0x2037, 0x2038, 0x2039,
                                                0x2040, 0x2041, 0x2042, 0x2043, 0x2044, 0x2045, 0x2046, 0x2047, 0x2048, 0x2049};
                                                
static vsomeip::eventgroup_t service_eventgroup_id_list[] = {0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009,
                                                                0x2010, 0x2011, 0x2012, 0x2013, 0x2014, 0x2015, 0x2016, 0x2017, 0x2018, 0x2019,
                                                                0x2020, 0x2021, 0x2022, 0x2023, 0x2024, 0x2025, 0x2026, 0x2027, 0x2028, 0x2029,
                                                                0x2030, 0x2031, 0x2032, 0x2033, 0x2034, 0x2035, 0x2036, 0x2037, 0x2038, 0x2039,
                                                                0x2040, 0x2041, 0x2042, 0x2043, 0x2044, 0x2045, 0x2046, 0x2047, 0x2048, 0x2049};

static vsomeip::service_t service_id_server;
static vsomeip::eventgroup_t service_eventgroup_id_server;

static vsomeip::service_t service_id_client;
static vsomeip::service_t service_eventgroup_id_client;

static vsomeip::event_t service_event_id = 0x0001;
static vsomeip::instance_t service_instance_id = 0x0001;
static vsomeip::major_version_t major_version = 0x01;
static vsomeip::minor_version_t minor_version = 0x00000001;


#endif // VSOMEIP_SERVICE_IDS_HPP