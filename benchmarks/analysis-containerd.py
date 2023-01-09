#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3
import os
from argparse import ArgumentParser
import numpy as np

SEARCH_STRING = "successfully booted in"

def main():
    parser = ArgumentParser(description="Get min, max, avg start time for a given test")
    parser.add_argument("-d", "--directory", dest="dir", type=str, required=True, 
                        help="directory of test log files")

    args = parser.parse_args()

    args.dir = os.path.abspath(args.dir)

    times_ls = []
    for file in os.listdir(args.dir):
        with open(args.dir + "/" + file) as f:
            for l in f.readlines():
                if l.find(SEARCH_STRING) != -1:
                    print('Value is: ' + l[-11:-3])
                    times_ls.append(float(l[-11:-3]))

    print('Min: ' + str(np.min(times_ls)))
    print('Max: ' + str(np.max(times_ls)))
    print('Avg: ' + str(np.mean(times_ls)))

if __name__ == "__main__":
    main()