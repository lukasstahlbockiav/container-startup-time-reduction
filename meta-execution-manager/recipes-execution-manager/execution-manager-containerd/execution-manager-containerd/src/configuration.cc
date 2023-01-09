/**
**  Created by Lukas Stahlbock in 2022
**  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
**/
#include <iostream>
#include <fstream>

#include "configuration.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void parse_execution_config_containers(const std::string& file_path, struct config_containers_s config_containers[], int& num){
    std::ifstream file_in(file_path);
    json j;
    file_in >> j;
    int i,k;
    for (auto& ctr : j["containers"].items())
    {
        //std::cout << "container " << ctr.key() << " : " << ctr.value() << std::endl;
        i = atoi(ctr.key().c_str());
        for (auto& el : ctr.value().items())
        {
            //std::cout << el.key() << " : " << el.value() << std::endl;
            if (el.key() == "name") {
                config_containers[i].name = el.value();
            } else if (el.key() == "command") {
                config_containers[i].command = el.value();
            } else if (el.key() == "digest") {
                config_containers[i].digest = el.value();
            } else if (el.key() == "image") {
                config_containers[i].image = el.value();
            } else if (el.key() == "oci_spec") {
                config_containers[i].oci_spec = el.value();
            } else if (el.key() == "runtime") {
                config_containers[i].runtime = el.value();
            } else if (el.key() == "mount_type") {
                config_containers[i].mount_type = el.value();
            } else if (el.key() == "mount_source") {
                config_containers[i].mount_source = el.value();
            } else if (el.key() == "mount_target") {
                config_containers[i].mount_target = el.value();
            } else if (el.key() == "mount_options") {
                for (auto& el_mo : el.value().items()){
                    k = atoi(el_mo.key().c_str());
                    config_containers[i].mount_options[k] = el_mo.value();
                }
            } else {
                std::cout << "unknown key: " << el.key() << std::endl;
            }
        }
    }
    num = i + 1;
};
