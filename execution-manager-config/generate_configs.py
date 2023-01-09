#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3

import json
import os
import time
from argparse import ArgumentParser
from copy import deepcopy
from common.helper import save_config, get_digest_for_image, get_content_for_digest, get_content_label_for_digest, get_mount_for_snapshot

CONFIG_TYPES_CRUN = ["mount", "umount", "create", "start", "kill", "delete", "run"]
CONFIG_TYPES_CTRD = ["prepare","create", "start", "run", "killAndDeleteContainer", "killAndDeleteTask"]
CONFIG_TEMPLATE = json.loads('{"containers":[]}')
CONFIG_RUNTIME = "io.containerd.runtime.v1.linux"

class Config_Base:
    def __init__(self, name, command, **kwargs):
        self.name = name
        self.command = command

class Config_Umount(Config_Base):
    def __init__(self, mount_target, **kwargs):
        self.mount_target = mount_target
        super().__init__(**kwargs)

class Config_Mount(Config_Umount):
    def __init__(self, mount_options, **kwargs):
        self.mount_options = mount_options
        super().__init__(**kwargs)

class Config_Bundle(Config_Base):
    def __init__(self, bundle, **kwargs):
        self.bundle = bundle
        super().__init__(**kwargs)

class Config_Run(Config_Mount, Config_Bundle):
    pass

class Config_Prepare_Ctrd(Config_Base):
    def __init__(self, digest, image, oci_spec, **kwargs):
        self.runtime = CONFIG_RUNTIME
        self.digest = digest
        self.image = image
        self.oci_spec = oci_spec
        super().__init__(**kwargs)

class Config_Run_Ctrd(Config_Base):
    def __init__(self, mount_type, mount_source, mount_target, mount_options, **kwargs):
        self.mount_type = mount_type
        self.mount_source = mount_source
        self.mount_target = mount_target
        self.mount_options = mount_options
        super().__init__(**kwargs)

class Config_Run_Native(Config_Base):
    def __init__(self, config, **kwargs):
        self.config = config
        super().__init__(**kwargs)

def get_mounts(layer_dir, bundle_dir, name):
    with open(layer_dir + "/layer_config.json") as f:
        layer_config = json.load(f)

    lower_dir = "lowerdir="
    upper_dir = "upperdir=" + layer_dir + "/sha256/" + name
    work_dir = "workdir=" + bundle_dir + "/work"
    for layer in layer_config[:-1]:
        lower_dir = lower_dir + layer_dir + "/" + layer['digest'].replace(':','/') + ":"
    
    lower_dir = lower_dir[:-1]
    mount_options = lower_dir + "," + upper_dir + "," + work_dir
    mount_target = bundle_dir + "/rootfs"

    return mount_options, mount_target

def create_config_crun(num, config_type, bundles_base_dir):
    config = deepcopy(CONFIG_TEMPLATE)
    for i in range(num):
        name = str(i).zfill(2)
        bundle = bundles_base_dir + "/" + name 
        #mount_options, mount_target = get_mounts(layer_dir, bundle, name)
        mount_type, mount_source, mount_target, mount_options = get_mount_command_from_snapshot(name)
        mount_target = bundle + "/rootfs"
        if config_type == "mount":
            config_temp = Config_Mount(mount_options=mount_options, mount_target=mount_target, name=name, command=config_type)
        elif config_type == "umount":
            config_temp = Config_Umount(mount_target=mount_target, name=name, command=config_type)
        elif config_type == "create":
            config_temp = Config_Bundle(bundle=bundle, name=name, command=config_type)
        elif config_type == "run":
            config_temp = Config_Run(bundle=bundle, mount_options=mount_options, mount_target=mount_target, name=name, command=config_type)
        else:
            config_temp = Config_Base(name=name, command=config_type)
        
        config['containers'].append(json.loads(json.dumps(config_temp.__dict__)))
    
    return config

def get_upper_snapshot_digest(image, output_dir, config_type, create_from_scratch_flag):
    upper_snapshot_digest = None
    if create_from_scratch_flag:
        while (upper_snapshot_digest == None or upper_snapshot_digest == ''):
            print("Determine upper layer snapshot digest")
            image_digest = get_digest_for_image(image)
            config_digest = get_content_for_digest(image_digest)['config']['digest']
            upper_snapshot_digest = get_content_label_for_digest(config_digest, 'containerd.io/gc.ref.snapshot.overlayfs')
            time.sleep(1)
    else:
        config_file = output_dir + "/execution-manager-containerd/" + config_type + ".json"
        with open(config_file, 'r', encoding='utf-8') as f:
            d = json.load(f)
            upper_snapshot_digest = d['containers'][0]['digest']
    
    return upper_snapshot_digest

def get_mount_command_from_snapshot(name):
    data = get_mount_for_snapshot(name)
    mount_type = "overlay"
    mount_source = "overlay"
    mount_target = "overlay"
    mount_options = data[-1].replace('\\n\'', "")
    #print(mount_options)
    return mount_type, mount_source, mount_target, mount_options

def create_config_ctrd(num, config_type, bundles_base_dir, image, create_from_scratch_flag, output_dir):
    config = deepcopy(CONFIG_TEMPLATE)
    
    for i in range(num):
        name = str(i).zfill(2)
        if config_type == "prepare":
            upper_snapshot_digest=get_upper_snapshot_digest(image, output_dir, config_type, create_from_scratch_flag)
            config_temp = Config_Prepare_Ctrd(digest=upper_snapshot_digest, image=image, oci_spec=bundles_base_dir + "/" + name + "/config.json", name=name, command=config_type)
        elif config_type == "create" or config_type == "run":
            mount_type, mount_source, mount_target, mount_options = get_mount_command_from_snapshot(name)
            mount_options = mount_options.split(',')
            config_temp = Config_Run_Ctrd(mount_type=mount_type, mount_source=mount_source, mount_target=mount_target, mount_options=mount_options, name=name, command=config_type)
        else:
            config_temp = Config_Base(name=name, command=config_type)

        config['containers'].append(json.loads(json.dumps(config_temp.__dict__)))
            
    return config

def create_config_native(num, config_type, bundles_base_dir):
    config = deepcopy(CONFIG_TEMPLATE)
    for i in range(num):
        name = str(i).zfill(2)
        config_dir = bundles_base_dir + '/' + name + '/config.json'
        config_temp = Config_Run_Native(config=config_dir, name=name, command=config_type)
        config['containers'].append(json.loads(json.dumps(config_temp.__dict__)))

    return config

def main():
    parser = ArgumentParser(description="Generate configurations for execution-manager")
    parser.add_argument("-n", "--num", dest="num", type=int, required=True, 
                        help="number of containers")
    parser.add_argument("-b", "--bundles", dest="bundles", type=str, required=True, 
                        help="base directory for all bundles")
    parser.add_argument("-c", "--config-type", dest="config_type", type=str, required=True, default="all",
                        help="config type to generate")
    parser.add_argument("-o", "--output-dir", dest="output_dir", type=str, required=True,
                        help="output directory for configs")
    parser.add_argument("-i", "--image", dest="image", type=str, required=False, 
                        help="specify used image")
    parser.add_argument("-t", "--target", dest="target", type=str, required=True,
                        help="specify target runtime [containerd, crun, native, all]")
    parser.add_argument("-s", "--scratch", dest="scratch", required=False, default=True, action='store_false',
                        help="[CONTAINERD ONLY] if flag is set an existing config will be parsed and elements in it will be copied")

    args = parser.parse_args()
    
    args.bundles = os.path.abspath(args.bundles)
    args.output_dir = os.path.abspath(args.output_dir)

    if args.scratch == None:
        args.scratch = True

    if args.target == 'containerd' or args.target == 'all':
        if args.config_type != 'all':
            config = create_config_ctrd(args.num, args.config_type, args.bundles, args.image, args.scratch, args.output_dir)
            save_config(config, args.config_type, args.output_dir + '/execution-manager-containerd')
        else:
            for config_type in CONFIG_TYPES_CTRD:
                config = create_config_ctrd(args.num, config_type, args.bundles, args.image, args.scratch, args.output_dir)
                save_config(config, config_type, args.output_dir + '/execution-manager-containerd')

    elif args.target == 'crun' or args.target == 'all':
        if args.config_type != 'all':
            config = create_config_crun(args.num, args.config_type, args.bundles)
            save_config(config, args.config_type, args.output_dir + '/execution-manager-crun')
        else:
            for config_type in CONFIG_TYPES_CRUN:
                config = create_config_crun(args.num, config_type, args.bundles)
                save_config(config, config_type, args.output_dir + '/execution-manager-crun')

    elif args.target == 'native' or args.target == 'all':
        config = create_config_native(args.num, args.config_type, args.bundles)
        save_config(config, 'run', args.output_dir + '/execution-manager-native')
    
if __name__ == "__main__":
    main()