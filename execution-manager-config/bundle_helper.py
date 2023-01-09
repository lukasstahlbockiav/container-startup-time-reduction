#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3

import os
import json
from argparse import ArgumentParser
from copy import deepcopy
from posixpath import split
from common.helper import make_directory, save_config

CONFIG_TYPES = ["isolated", "host", "pod", "native"]

def prepare_bundle_directory(bundle_dir):
    bundle_configs_dir = bundle_dir + "/configs"
    bundle_rootfs_dir = bundle_dir + "/rootfs"
    bundle_work_dir = bundle_dir + "/work"
    make_directory(bundle_dir)
    make_directory(bundle_configs_dir)
    make_directory(bundle_rootfs_dir)
    make_directory(bundle_work_dir)

def prepare_bundle_configs(config_template, name, config_dir, cpus):
    config = deepcopy(config_template)

    config['process']['env'][2] = "VSOMEIP_LOGGING_PATH=/tmp/logs/vsomeip-client-" + name + ".log"
    config['process']['env'][3] = "VSOMEIP_APPLICATION_NAME=client-" + name
    config['process']['env'][4] = "VSOMEIP_APPLICATION_ID=0x10" + name
    config['process']['env'][5] = "VSOMEIP_CLIENT=0"
    config['process']['env'][6] = "VSOMEIP_STRESS_CPU=0"
    config['process']['env'][7] = "VSOMEIP_STRESS_CPU_ITERATIONS=0"

    for config_type in CONFIG_TYPES:
        config_temp = deepcopy(config)
        if config_type == "host":
            config_temp['linux']['namespaces'] = [{"type":"mount"}]
        elif config_type == "pod":
            config_temp['linux']['namespaces'][0] = {"type": "pid", "path": "/proc/XXX/ns/pid"}
            config_temp['linux']['namespaces'][1] = {"type": "network", "path": "/proc/XXX/ns/net"}
            config_temp['linux']['namespaces'][2] = {"type": "ipc", "path": "/proc/XXX/ns/ipc"}
        elif config_type == "native":
            native_config = {}
            native_config['process'] = {}
            native_config['process']['env'] = config_temp['process']['env']
            config_temp = deepcopy(native_config)
        
        if cpus != "":
            config['linux']['cpu']['cpus'] = cpus
        
        save_config(config_temp, config_type, config_dir)

def modify_config(config, pid):
    config['linux']['namespaces'][0] = {"type": "pid", "path": "/proc/" + pid + "/ns/pid"}
    config['linux']['namespaces'][1] = {"type": "network", "path": "/proc/" + pid + "/ns/net"}
    config['linux']['namespaces'][2] = {"type": "ipc", "path": "/proc/" + pid + "/ns/ipc"}

    return config
    

def main():
    parser = ArgumentParser(description="Setup bundle directories for execution-manager benchmarks")
    parser.add_argument("-b", "--bundles", dest="bundles", type=str, required=True, 
                        help="directory of bundles")
    parser.add_argument("-n", "--num", dest="num", type=int, required=True, 
                        help="number of bundles")
    parser.add_argument("-m", "--mode", dest="mode", type=str, required=True, 
                        help="usage mode [prepare, modify]")
    parser.add_argument("-c", "--config", dest="config", type=str, required=False, 
                        help="prepare: config.json template \n modify: not required")
    parser.add_argument("-p","--pid", dest="pid", type=str, required=False,
                        help="update pid for pid, net, ipc namespaces")
    parser.add_argument("--cpus", dest="cpus", type=str, required=False,
                        help="comma seperated list of cpu cores to use, e.g. 0,1,5")

    args = parser.parse_args()

    args.bundles = os.path.abspath(args.bundles)

    if args.mode == 'prepare':
        args.config = os.path.abspath(args.config)
        print("open: "+ args.config)
        with open(args.config, 'r+') as f:
            config_template = json.load(f)

        for i in range(args.num):
            name = str(i).zfill(2)
            bundle_dir = args.bundles + "/" + name
            prepare_bundle_directory(bundle_dir)
            prepare_bundle_configs(config_template, name, bundle_dir +"/configs", args.cpus)
    
    elif args.mode == 'modify':
        for i in range(args.num):
            name = str(i).zfill(2)
            config_dir = args.bundles + "/" + name
            with open(config_dir + "/config.json") as f:
                config = json.load(f)

            config = modify_config(config, args.pid)
            save_config(config, "config", config_dir)

if __name__ == "__main__":
    main()