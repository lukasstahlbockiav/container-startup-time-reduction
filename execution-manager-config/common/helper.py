#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3

import os
import json
import subprocess

def make_directory(path):
    try:
        os.mkdir(path)
    except OSError:
        print ("Creation of the directory %s failed" % path)

def save_config(config, config_type, config_dir):
    if not os.path.isdir(config_dir):
        make_directory(config_dir)
    
    #print(json.dumps(config, indent=4))
    with open(config_dir + "/" + config_type + ".json", 'w', encoding='utf-8') as f:
        json.dump(config, f, ensure_ascii=False, indent=4)

def call_binary(args, stdout):
    if stdout:
        popen = subprocess.Popen(args, stdout=subprocess.PIPE)
        popen.wait()
        return popen.stdout.read()
    else:
        FNULL = open(os.devnull, 'w')
        popen = subprocess.Popen(args, stdout=FNULL)
        popen.wait()
        return 1

def get_digest_for_image(image):
    args = ("containerd-ctr", "image", "ls", "name==" + image)
    result = call_binary(args, True)

    data = str(result).split()
    for substring in data:
        if substring.find("sha256") != -1:
            return substring

def get_content_for_digest(digest):
    args = ("containerd-ctr", "content", "get", digest)
    result = call_binary(args, True)
    
    data = json.loads(result)
    return data

def get_content_label_for_digest(digest, label):
    args = ("containerd-ctr", "content", "ls", "digest==" + digest)
    result = call_binary(args, True)

    data = str(result).split()
    for substring in data:
        if substring.find(label) != -1:
            ind_label = substring.find(label) + len(label) + 1
            digest = substring[ind_label:ind_label+substring[ind_label:].find(',')]
            return digest

def get_mount_for_snapshot(name):
    args = ("containerd-ctr", "snapshots", "mount", "overlay", name)
    result = call_binary(args,True)
    
    data = str(result).split()

    return data
