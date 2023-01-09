#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3
import os
import time
import datetime
import pandas as pd
import numpy as np
from argparse import ArgumentParser

# directory structure: test/runtime/number/iteration
START_STRINGS = [['crun', 'run container'], 
                    ['crun_batch', 'run container'], 
                    ['ctrd', '/tasks/create'], 
                    ['native', 'run application']]
START_STRINGS_SEQUENTIELL = [['crun', ' mount ', ' create ', ' start '], 
                                ['crun_batch', ' mount ', ' create ', ' start '], 
                                ['ctrd', 'dummy mount string', ' /tasks/create ', ' /tasks/start '], 
                                ['native', 'dummy mount string', 'dummy create string',' run application ']]
END_STRING_SEQUENTIELL = 'finished'
END_STRING = 'is registered'
DUMMY_BASE_TIME = "2021-07-19T"

def convert_datetime_to_timestamp(datetime_str):
    datetime_str = DUMMY_BASE_TIME + datetime_str[11:26]
    dt = datetime.datetime.strptime(datetime_str, "%Y-%m-%dT%H:%M:%S.%f")
    timestamp = time.mktime(dt.timetuple()) + (dt.microsecond / 1000000.0)
    #print(timestamp)
    return timestamp

def get_current_runtime_number_iteration(path, base_dir):
    rel_path = path[len(base_dir)+1:]
    #print(rel_path)
    runtime, num, iteration = rel_path.split("/")

    return runtime, num, iteration

def get_start_string(runtime, mapping_list):
    for e in mapping_list:
        if e[0] == runtime:
            return e[1]

def get_operation_strings(runtime, mapping_list):
    for e in mapping_list:
        if e[0] == runtime:
            return e
    
def get_timestamp(filepath, search_string):
    with open(filepath) as f:
        for l in f.readlines():
            if l.find(search_string) != -1:
                return convert_datetime_to_timestamp(l)
        return 0

def get_last_timestamp(filepath, search_string):
    with open(filepath) as f:
        for l in reversed(f.readlines()): # read files from 
            if l.find(search_string) != -1:
                return convert_datetime_to_timestamp(l)
        return 0

def calculate_statistical_values(ls):
    avg = np.mean(ls)
    min = np.min(ls)
    max = np.max(ls)
    return avg, min, max

def main():
    parser = ArgumentParser(description="Get min, max, avg start time for a given test")
    parser.add_argument("-d", "--directory", dest="dir", type=str, required=True, 
                        help="directory of test log files")

    args = parser.parse_args()

    args.dir = os.path.abspath(args.dir)
    test_name = os.path.basename(os.path.normpath(args.dir))
    # delete old csv file(s)
    files = os.listdir(args.dir)
    filtered_files = [file for file in files if file.endswith(".csv")]
    for file in filtered_files:
        path_to_file = os.path.join(args.dir, file)
        os.remove(path_to_file)
    
    df = pd.DataFrame()
    df_runtime_ls = []
    diff_ls = []
    diff_mount_ls = []
    diff_create_ls = []
    diff_start_ls = []
    last_num = ''
    n_iter = 0
    c_iter = 0
    i_runtime = 0
    for path, dirs, files in os.walk(args.dir):
        print(path)
        print(dirs)
        print(files)
        if dirs and path.count('/') - args.dir.count('/') == 1:
            df_runtime_ls.append(pd.DataFrame())
            i_runtime = len(df_runtime_ls) - 1

        elif dirs and path.count('/') - args.dir.count('/') == 2:
            #print(dirs)
            n_iter = len(dirs)
            c_iter = 0
            print('Number of iterations are: ' + str(n_iter))
                       
        if test_name.find('sequentiell') != -1 and files:
            runtime, num, iteration = get_current_runtime_number_iteration(path, args.dir)
            runtime_no_underscore = runtime.replace('_',' ')
            
            #print(str(c_iter) + ': Parsing iteration ' + iteration + ' for ' + num + ' containers for runtime ' + runtime)
            c_iter = c_iter + 1
            last_num = num
            operation_strings = get_operation_strings(runtime, START_STRINGS_SEQUENTIELL)
            mount_string = operation_strings[1]
            create_string = operation_strings[2]
            start_string = operation_strings[3]
            mount_ts_start = 0
            mount_ts_end = 0
            create_ts_start = 0
            create_ts_end = 0
            start_ts_start = 0
            start_ts_end = 0
            
            for file in files:
                filepath = path + "/" + file
                if file.find(runtime) != -1:
                    mount_ts_start = get_timestamp(filepath, mount_string)
                    mount_ts_end = get_last_timestamp(filepath, mount_string)
                    create_ts_start = get_timestamp(filepath, create_string)
                    create_ts_end = get_last_timestamp(filepath, create_string)
                    start_ts_start = get_timestamp(filepath, start_string)
                else:
                    start_ts_end_temp = get_timestamp(filepath, END_STRING)
                    if start_ts_end_temp is not None and start_ts_end_temp > start_ts_end:
                        start_ts_end = start_ts_end_temp
                
            diff_mount_ls.append(mount_ts_end - mount_ts_start)
            diff_create_ls.append(create_ts_end - create_ts_start)
            diff_start_ls.append(start_ts_end - start_ts_start)

            if c_iter == n_iter and diff_mount_ls and diff_create_ls and diff_start_ls:
                print('Parsed ' + num + ' containers for runtime ' + runtime)
                
                val_mount_avg, val_mount_min, val_mount_max = calculate_statistical_values(diff_mount_ls)
                val_create_avg, val_create_min, val_create_max = calculate_statistical_values(diff_create_ls)
                val_start_avg, val_start_min, val_start_max = calculate_statistical_values(diff_start_ls)
                print('Mount values are: ' + str(val_mount_min) + '|' + str(val_mount_avg) + '|' + str(val_mount_max))
                print('Create values are: ' + str(val_create_min) + '|' + str(val_create_avg) + '|' + str(val_create_max))
                print('Start values are: ' + str(val_start_min) + '|' + str(val_start_avg) + '|' + str(val_start_max))
                print('max values at: ', diff_mount_ls.index(val_mount_max), diff_create_ls.index(val_create_max), diff_start_ls.index(val_start_max))
                error_minus_mount = val_mount_avg-val_mount_min
                error_plus_mount =  val_mount_max-val_mount_avg
                error_minus_create = val_create_avg-val_create_min
                error_plus_create = val_create_max-val_create_avg
                error_minus_start = val_start_avg-val_start_min
                error_plus_start = val_start_max-val_start_avg
                error_minus = error_minus_mount + error_minus_create + error_minus_start
                error_plus = error_plus_mount + error_plus_create + error_plus_start
                values = np.array([[val_mount_min, val_mount_avg, val_mount_max,
                                    val_create_min, val_create_avg, val_create_max, 
                                    val_start_min, val_start_avg, val_start_max, 
                                    error_minus, error_plus]])
                df_iter = pd.DataFrame(values, columns=['min mount '+runtime_no_underscore,
                                                        'avg mount '+runtime_no_underscore,
                                                        'max mount '+runtime_no_underscore,
                                                        'min create '+runtime_no_underscore,
                                                        'avg create '+runtime_no_underscore,
                                                        'max create '+runtime_no_underscore,
                                                        'min start '+runtime_no_underscore,
                                                        'avg start '+runtime_no_underscore,
                                                        'max start '+runtime_no_underscore,
                                                        'error minus '+runtime_no_underscore,
                                                        'error plus '+runtime_no_underscore], index=[last_num])
                #print(df_iter)
                df_runtime_ls[i_runtime] = pd.concat([df_runtime_ls[i_runtime], df_iter], axis=0, join='outer')
                #print(df_runtime_ls)
                diff_mount_ls.clear()
                diff_create_ls.clear()
                diff_start_ls.clear()

                c_iter = 0
                        
        elif files:
            runtime, num, iteration = get_current_runtime_number_iteration(path, args.dir)
            runtime_no_underscore = runtime.replace('_',' ')
            #if runtime == 'ctrd':
            #    print(str(c_iter) + ': Parsing iteration ' + iteration + ' for ' + num + ' containers for runtime ' + runtime)
            c_iter = c_iter + 1
            last_num = num
            start_string = get_start_string(runtime, START_STRINGS)
            start_ts = 0
            end_ts = 0
            for file in files:
                filepath = path + "/" + file
                if file.find(runtime) != -1:
                    start_ts = get_timestamp(filepath, start_string)
                else:
                    end_ts_temp = get_timestamp(filepath, END_STRING)
                    if end_ts_temp is not None and end_ts_temp > end_ts:
                        end_ts = end_ts_temp
                
            diff_ls.append(end_ts - start_ts)
     
            if c_iter == n_iter and diff_ls:
                print('Parsed ' + num + ' containers for runtime ' + runtime)
                val_avg = np.mean(diff_ls)
                val_min = np.min(diff_ls)
                val_max = np.max(diff_ls)
                print('Values are: ' + str(val_min) + '|' + str(val_avg) + '|' + str(val_max))
                print('max value at: ', diff_ls.index(val_max))
                if np.isnan(val_avg) or np.isnan(val_min) or np.isnan(val_max):
                    print(diff_ls)
                values = np.array([[val_min, val_avg, val_max, val_avg-val_min, val_max-val_avg]])
                df_iter = pd.DataFrame(values, columns=['min '+runtime_no_underscore,
                                                        'avg '+runtime_no_underscore,
                                                        'max '+runtime_no_underscore,
                                                        'error minus '+runtime_no_underscore,
                                                        'error plus '+runtime_no_underscore], index=[last_num])
                #print(df_iter)
                df_runtime_ls[i_runtime] = pd.concat([df_runtime_ls[i_runtime], df_iter], axis=0, join='outer')
                #print(df_runtime_ls)
                diff_ls.clear()
                c_iter = 0

    df = pd.concat(df_runtime_ls, axis=1).sort_index()
    df.index.name = 'num ctr'
    df['num'] = range(1,len(df.index)+1)

    print(df)
    df.to_csv(args.dir + '/' + test_name + '.csv')

if __name__ == "__main__":
    main()