#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3

import json
import os
from argparse import ArgumentParser
from common.helper import make_directory, call_binary, get_digest_for_image, get_content_for_digest
import glob
from shutil import copy

def install_native(layer_dir):
    for layer in os.listdir(layer_dir):
        base_path = layer_dir + '/' + layer
        #print(base_path)
        files = glob.iglob(base_path + '/**', recursive=True)
        for file in files:
            #print(file)
            if os.path.isfile(file):
                relative_path = file[len(base_path):]
                if not os.path.isfile(relative_path):
                    print('copy file ' + file + ' to ' + relative_path)
                    os.makedirs(os.path.dirname(relative_path), exist_ok=True)
                    copy(file, relative_path, follow_symlinks=False)

def copy_upper_layer(layer_dir, upper_layer_digest, num):
    base_path = layer_dir + "/" + upper_layer_digest
    for i in range(num):
        files = glob.iglob(base_path + '/**', recursive=True)
        for file in files:
            #print(file)
            if os.path.isfile(file):
                relative_path = file[len(base_path):]
                target = layer_dir + "/" + str(i).zfill(2) + relative_path
                if not os.path.isfile(target):
                    print('copy file ' + file + ' to ' + target)
                    os.makedirs(os.path.dirname(target), exist_ok=True)
                    copy(file, target, follow_symlinks=False)
        

def extract_layers(layers, containerd_content_blobs, output_dir, native, num):
    last_digest = ""
    if not os.path.isdir(output_dir + "/sha256"):
        make_directory(output_dir + "/sha256")
    for layer in layers:
        layer_dir = output_dir + "/" + layer['digest'].replace(':','/')
        if not os.path.isdir(layer_dir):
            args = ("tar", "-zxvf", containerd_content_blobs + layer['digest'].replace(":","/"), "-C", layer_dir)
            make_directory(layer_dir)
            result = call_binary(args, False)
            
            print("Extracted layer to " + layer_dir)
            layer['parent'] = last_digest
        last_digest = layer['digest']
    
    with open(output_dir + "/layer_config.json", 'w', encoding='utf-8') as f:
        json.dump(layers, f, ensure_ascii=False, indent=4)
    
    if (native):
        install_native(output_dir + "/sha256")

    copy_upper_layer(output_dir + "/sha256", layers[-1]['digest'].replace("sha256:",""), num)


def main():
    
    parser = ArgumentParser(description="Extract all layers for a given containerd image")
    parser.add_argument("-i", "--image", dest="image_ref", type=str, required=True, 
                        help="image to extract layers from")
    parser.add_argument("-o", "--output", dest="output_dir", type=str, required=True, 
                        help="output directory for extracted layers")
    parser.add_argument("--copy-native", dest="native", type=bool, required=False, 
                        help="also copy all files from image dir to native host after extracting")
    parser.add_argument("-n", "--num", dest="num", type=int, required=True, 
                        help="number of times the upper layer should be copied")
    parser.add_argument("--content-blobs", dest="containerd_content_blobs", type=str, 
                        default="/var/lib/containerd/io.containerd.content.v1.content/blobs/", 
                        help="directory where containerd content blobs are stored")

    args = parser.parse_args()
    
    args.output_dir = os.path.abspath(args.output_dir)

    digest = get_digest_for_image(args.image_ref)
    print(digest)
    layers = get_content_for_digest(digest)['layers']
    print(layers)
    extract_layers(layers, args.containerd_content_blobs, args.output_dir, args.native, args.num)


if __name__ == "__main__":
    main()