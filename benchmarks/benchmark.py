#
#  Created by Lukas Stahlbock in 2022
#  Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
#

#/bin/env/python3

import os
import json
import subprocess
from argparse import ArgumentParser
import time
from shutil import copyfile

def set_core_states(core_list, disable_flag):
    state = "1"
    if disable_flag:
        state = "0"

    for core in core_list:
        args_echo = ("echo", state)
        args_tee = ("tee", "/sys/devices/system/cpu/" + core + "/online")
        echo = subprocess.Popen(args_echo, stdout=subprocess.PIPE)
        output = subprocess.check_output(args_tee, stdin=echo.stdout)
        echo.wait()
        #print(output)

def get_taskset_cores(taskset_cores):
    cores = ''
    for core in taskset_cores:
        cores = cores + core[-1] + ","
    
    return cores[:-1]

def set_cgroup_v2(mount_flag):
    if mount_flag:
        args = ("mount",  "-t", "cgroup2", "none",  "/sys/fs/cgroup")
    else:
        args = ("umount", "/sys/fs/cgroup")
    mount = subprocess.Popen(args, stdout=subprocess.PIPE)
    mount.wait()

def set_cgroup_v1(mount_flag):
    if mount_flag:
        args_mount = ("mount", "-t", "cgroup", "-o", "none,name=systemd", "cgroup", "/sys/fs/cgroup/systemd")
        os.makedirs("/sys/fs/cgroup/systemd", exist_ok=True)
    else:
        args_mount = ("umount", "/sys/fs/cgroup/systemd")      

    mount = subprocess.Popen(args_mount, stdout=subprocess.PIPE)
    mount.wait()

def run_daemon(daemon_config):
    open(daemon_config['log-uri'], 'w').close()

    args = ("containerd-ctr", "run",
            "--detach",
            "--rm", 
            "--log-uri", daemon_config['log-uri'],
            "--runtime", "io.containerd.runtime.v1.linux", 
            "--net-host",
            "--mount", "type=bind,src=/tmp/,dst=/tmp/,options=rbind:rw", 
            "--mount", "type=bind,src=" + daemon_config['config-dir'] + ",dst=/usr/local/etc/vsomeip/,options=rbind:rw",
            "--env", "VSOMEIP_CONFIGURATION=/usr/local/etc/vsomeip/vsomeip.json", 
            "--env", "LD_LIBRARY_PATH=/lib/aarch64-linux-gnu/:/usr/lib/aarch64-linux-gnu/:/usr/local/lib/:/lib/:/usr/lib/",
            daemon_config['image'], 
            "vsomeip-daemon")
    
    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()
    #print(ctr.stdout.read())

def run_pod(pod_config):
    args = ("crun", "run", "--detach", "-b", pod_config['bundle'], "--pid-file=/home/container-startup-time-reduction/benchmarks/pod_pid", "pod")

    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()
    with open("/home/container-startup-time-reduction/benchmarks/pod_pid", 'r+') as f:
        pid = f.read()
        return pid

def run_daemon_native(daemon_config):
    args = ("/usr/local/bin/routingmanagerd")
    env = {}
    env['VSOMEIP_CONFIGURATION'] = "/home/container-startup-time-reduction/configs/vsomeip.json"
    env['LD_LIBRARY_PATH'] = "/lib/aarch64-linux-gnu/:/usr/lib/aarch64-linux-gnu/:/usr/local/lib/:/lib/:/usr/lib/"
    logfile = open(daemon_config['log-uri'], 'w+')
    ctr = subprocess.Popen(args, env=env, stdout=logfile, stderr=logfile)
    #ctr.wait()
    return ctr

def kill_daemon():
    args = ("containerd-ctr", "tasks", "kill", "--signal", "SIGTERM", "vsomeip-daemon")
    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()

    args = ("containerd-ctr", "container", "delete", "vsomeip-daemon")
    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()

def kill_daemon_native(daemon):
    daemon.kill()

def kill_pod(config):
    args = ("crun", "kill", "pod")

    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()
    time.sleep(config['duration'])
    args = ("crun", "delete", "pod")

    ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    ctr.wait()

def prepare_image_and_bundle(config, pod_pid):
    num = max(config['num'])
    cpus = get_taskset_cores(config['taskset_cores'])
    args = ("taskset", "-c", cpus, "/usr/bin/docker-containerd")
    subprocess.Popen(args, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    # prepare image
    #args = ("containerd-ctr", "images", "pull", "-u", 
    #    config['ctr-credentials']['username'] + ":" + config['ctr-credentials']['token'],
    #    config['image'])
    #ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    #ctr.wait()
    # print(ctr.stdout.read())

    args = ("python3", "./../execution-manager-config/bundle_helper.py",
            "-b", config['bundle-dir'], "-n", str(num),
            "-m", "prepare", "-c", config['config-template'], "--cpus", cpus)
    bundle_helper = subprocess.Popen(args, stdout=subprocess.PIPE)
    bundle_helper.wait()

    for i in range(num):
        current_bundle_path = os.path.abspath(config['bundle-dir'] + "/" + str(i).zfill(2))
        copyfile(current_bundle_path + "/configs/" + config['oci-config'], current_bundle_path + "/config.json")
    
    if (config['oci-config'] == 'pod.json'):
        args = ("python3", "./../execution-manager-config/bundle_helper.py",
                "-b", config['bundle-dir'], "-n", str(num),
                "-m", "modify", "-p", pod_pid)
        bundle_helper = subprocess.Popen(args, stdout=subprocess.PIPE)
        bundle_helper.wait()

    for i in range(num):
        args = ("containerd-ctr", "c", "create", "--runtime", "io.containerd.runtime.v1.linux", 
                "--env", "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
                "--env", "VSOMEIP_CONFIGURATION=/usr/local/etc/vsomeip/vsomeip-default.json",
                "--env", "VSOMEIP_LOGGING_PATH=/tmp/logs/vsomeip-client-"+ str(i).zfill(2) +".log",
                "--env", "VSOMEIP_APPLICATION_NAME=client-" + str(i).zfill(2),
                "--env", "VSOMEIP_APPLICATION_ID=0x10" + str(i).zfill(2),
                "--env", "VSOMEIP_CLIENT=0",
                "--env", "VSOMEIP_STRESS_CPU=0",
                "--env", "VSOMEIP_STRESS_CPU_ITERATIONS=0",
                "--env", "LD_LIBRARY_PATH=/lib/aarch64-linux-gnu/:/usr/lib/aarch64-linux-gnu/:/usr/local/lib/:/lib/:/usr/lib/",
                "--cwd", "/usr/local/bin",
                "--mount", "type=bind,src=/tmp,dst=/tmp,options=rbind:rw",
                config['image'], str(i).zfill(2))
        prepare = subprocess.Popen(args, stdout=subprocess.PIPE)
        prepare.wait()

    # base config only for containerd. We must get the upper layer digest snapshot digest only once
    # later on (in test_loop) we just copy this digest instead of asking containerd again
    args = ("python3", "./../execution-manager-config/generate_configs.py",
            "-i", config['image'], "-n", "1", "-b", config['bundle-dir'],
            "-t", "containerd", "-o", config['config-dir'], "-c", "all")
    create_test_config(args)

    # prepare daemon
    #args = ("containerd-ctr", "images", "pull", "-u", 
    #    config['ctr-credentials']['username'] + ":" + config['ctr-credentials']['token'],
    #    config['daemon']['image'])
    #ctr = subprocess.Popen(args, stdout=subprocess.PIPE)
    #ctr.wait()

def unprepare(config):
    num = max(config['num'])

    for i in range(num):
        args = ("containerd-ctr", "c", "delete", str(i).zfill(2))
        prepare = subprocess.Popen(args, stdout=subprocess.PIPE)
        prepare.wait()
    
    args = ("/bin/bash", "-c", "kill $(ps | grep '[/]usr/bin/docker-containerd' | awk '{print $1}')")
    subprocess.Popen(args)

    for i in range(num):
        current_bundle_path = os.path.abspath(config['bundle-dir'] + "/" + str(i).zfill(2))
        os.remove(current_bundle_path + "/config.json")

def create_test_config(args):
    generate_config = subprocess.Popen(args, stdout=subprocess.PIPE)
    generate_config.wait()

def run(config, exec_type, exec_id):
    taskset_cores = get_taskset_cores(config['taskset_cores'])
    if exec_type == "ctrd":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "-c", config['config-dir'] + "/execution-manager-containerd/run.json")
        logfile = open('logs/ctrd.log', 'w+')
    elif exec_type == "crun":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/run.json")
        logfile = open('logs/crun.log', 'w+')
    elif exec_type == "crun_batch":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/run.json")
        logfile = open('logs/crun_batch.log', 'w+')
    elif exec_type == 'native':
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "--native", config['config-dir'] + "/execution-manager-native/run.json")
        logfile = open('logs/native.log', 'w+')

    run = subprocess.Popen(args, stdout=logfile, stderr=logfile)
    run.wait()
    logfile.close()

def run_sequentiell(config, exec_type, exec_id):
    taskset_cores = get_taskset_cores(config['taskset_cores'])
    if exec_type == "ctrd":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "-c", config['config-dir'] + "/execution-manager-containerd/create.json")
        logfile = open('logs/ctrd.log', 'w+')
    elif exec_type == "crun":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/mount.json")
        logfile = open('logs/crun.log', 'w+')
    elif exec_type == "crun_batch":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/mount.json")
        logfile = open('logs/crun_batch.log', 'w+')
    elif exec_type == 'native':
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "--native", config['config-dir'] + "/execution-manager-native/run.json")
        logfile = open('logs/native.log', 'w+')

    run = subprocess.Popen(args, stdout=logfile, stderr=logfile)
    run.wait()
    logfile.close()

    if exec_type == "ctrd":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "-c", config['config-dir'] + "/execution-manager-containerd/start.json")
        logfile = open('logs/ctrd.log', 'a+')
    elif exec_type == "crun":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/create.json")
        logfile = open('logs/crun.log', 'a+')
    elif exec_type == "crun_batch":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/create.json")
        logfile = open('logs/crun_batch.log', 'a+')

    if exec_type == "ctrd" or exec_type == "crun" or exec_type == "crun_batch":
        run = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        run.wait()
        logfile.close()

    if exec_type == "crun":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/start.json")
        logfile = open('logs/crun.log', 'a+')
    elif exec_type == "crun_batch":
        args = ("taskset", "-c", taskset_cores, config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/start.json")
        logfile = open('logs/crun_batch.log', 'a+')

    if exec_type == "crun" or exec_type == "crun_batch":
        run = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        run.wait()
        logfile.close()

def kill(config, exec_type, exec_id):
    if exec_type == "ctrd":
        args = (config['executables'][exec_id]['path'], "-c", config['config-dir'] + "/execution-manager-containerd/killAndDeleteTask.json")
        logfile = open('logs/ctrd.log', 'a')
    elif exec_type == "crun":
        args = (config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/kill.json")
        logfile = open('logs/crun.log', 'a')
    elif exec_type == "crun_batch":
        args = (config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/kill.json")
        logfile = open('logs/crun_batch.log', 'a')
    elif exec_type == 'native':
        #kill $(ps aux | grep '[v]someip_generic' | awk '{print $2}')
        cmd = "kill -TERM $(ps aux | grep '[v]someip-generic' | awk '{print $1}')"
        args = ("/bin/bash", "-c", cmd)
        logfile = open('logs/native.log', 'a')
    
    logfile.flush()
    kill = subprocess.Popen(args, stdout=logfile, stderr=logfile)
    kill.wait()
    time.sleep(1)
    logfile.close()

    if exec_type == "crun":
        time.sleep(1) # sometimes crun needs some more to actually set container state to stopped after kill signal was sent
        args = (config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/delete.json")
        logfile = open('logs/crun.log', 'a')
        logfile.flush()
        delete = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        delete.wait()
        time.sleep(1) # try waiting some time until containers are really deleted
        args = (config['executables'][exec_id]['path'], "--debug", config['config-dir'] + "/execution-manager-crun/umount.json")
        umount = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        umount.wait()
        logfile.close()
    elif exec_type == "crun_batch":
        time.sleep(1) # sometimes crun needs some more to actually set container state to stopped after kill signal was sent
        args = (config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/delete.json")
        logfile = open('logs/crun_batch.log', 'a')
        logfile.flush()
        delete = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        delete.wait()
        time.sleep(1) # try waiting some time until containers are really deleted
        args = (config['executables'][exec_id]['path'], "--debug", "batch", config['config-dir'] + "/execution-manager-crun/umount.json")
        umount = subprocess.Popen(args, stdout=logfile, stderr=logfile)
        umount.wait()
        logfile.close()
    #elif exec_type == "ctrd":
    #    time.sleep(1)
    #    args = ("containerd-ctr", "c", "ls")
    #    ls = subprocess.Popen(args, stdout=subprocess.PIPE)
    #    ls.wait()
    #    for line in ls.stdout.readlines():
    #        print(line)
    #        data = str(line).replace('b\'','').split()
    #        id = data[0]
    #        print("substring is: " + id)
    #        if len(id) == 2:
    #            print("Deleting " + id)
    #            args = ("containerd-ctr", "c", "delete", id)
    #            logfile = open('logs/ctrd.log', 'a')
    #            logfile.flush()
    #            delete = subprocess.Popen(args, stdout=logfile, stderr=logfile)
    #            delete.wait()
    #    
    #    snapshots_exist = True
    #    while (snapshots_exist):
    #        snapshots_exist = False
    #        args = ("containerd-ctr", "snapshots", "ls")
    #        ls = subprocess.Popen(args, stdout=subprocess.PIPE)
    #        ls.wait()
    #        for line in ls.stdout.readlines():
    #            print(line)
    #            data = str(line).replace('b\'','').split()
    #            id = data[0]
    #            if len(id) == 2:
    #                snapshots_exist = True
    #                print("Remove snapshot " + id)
    #                args = ("containerd-ctr", "snapshots", "rm", id)
    #                logfile = open('logs/ctrd.log', 'a')
    #                logfile.flush()
    #                delete = subprocess.Popen(args, stdout=logfile, stderr=logfile)
    #                delete.wait()
    #                break
    #
    #       time.sleep(2)
                    

            

def test_loop(config):
    log_base = '/tmp/logs'
    os.makedirs(log_base, exist_ok=True)
    test_runtimes = []
    for i_exec, exec in enumerate(config['executables']):
        if exec['type'] == "containerd":
            test_runtimes.append(["ctrd", i_exec])
        elif exec['type'] == "crun":
            test_runtimes.append(["crun", i_exec])
        elif exec['type'] == "crun_batch":
            test_runtimes.append(["crun_batch", i_exec])
        elif exec['type'] == "native":
            test_runtimes.append(["native", i_exec])

    for num in config['num']:
        print("create configs for " + str(num) + " containers")
        for runtime in test_runtimes:
            if runtime[0] == "ctrd":
                args = ("python3", "./../execution-manager-config/generate_configs.py",
                        "-i", config['image'], "-n", str(num), "-b", config['bundle-dir'],
                        "-t", "containerd", "-o", config['config-dir'], "-c", "all", "-s")
                create_test_config(args)

            if runtime[0] == "crun" or runtime[0] == "crun_batch":
                args = ("python3", "./../execution-manager-config/generate_configs.py",
                        "-n", str(num), "-b", config['bundle-dir'],
                        "-t", "crun", "-o", config['config-dir'], "-c", "all")
                create_test_config(args)
            
            if runtime[0] == "native":
                args = ("python3", "./../execution-manager-config/generate_configs.py",
                        "-n", str(num), "-b", config['bundle-dir'],
                        "-t", "native", "-o", config['config-dir'], "-c", "/usr/local/bin/vsomeip-generic")
                create_test_config(args)

        for iteration in range(0,config['iterations']):
            print("iteration " + str(iteration))
            for runtime in test_runtimes:
                log_target = config['log-dir'] + '/'  + config['test-name'] + '/' + runtime[0] + '/' + str(num).zfill(2) + '/' + str(iteration).zfill(2)
                os.makedirs(log_target, exist_ok=True)
            
                for i in range(num):
                    f = open(log_base + '/vsomeip-client-' + str(i).zfill(2) + '.log', 'w+')
                    f.close()
                if config['run-type'] == 'run-sequentiell':
                    run_sequentiell(config, runtime[0], runtime[1])
                else:
                    run(config, runtime[0], runtime[1])
                time.sleep(config['duration'])
                kill(config, runtime[0], runtime[1])
                for i in range(num):
                    copyfile(log_base + '/vsomeip-client-' + str(i).zfill(2) + '.log',
                                log_target + '/vsomeip-client-' + str(i).zfill(2) + '.log')
                copyfile(config['log-dir'] + '/' + runtime[0] + '.log',
                            log_target + '/' + runtime[0] + '.log')
              

def main():
    parser = ArgumentParser(description="Setup bundle directories for execution-manager benchmarks")
    parser.add_argument("-c", "--config", dest="config", type=str, required=True, 
                        help="test configuration file")

    args = parser.parse_args()

    args.config= os.path.abspath(args.config)

    with open(args.config, 'r+') as f:
        config = json.load(f)

    if config['cgroup-version'] == 'v1':
        #set_cgroup_v2(True)
        set_cgroup_v1(True)
    
    pod_pid = ''
    if config['pod']['run'] == "true":
        pod_pid = run_pod(config['pod'])

    prepare_image_and_bundle(config, pod_pid)
    #if config['prepare'] == "true":
    #    if not os.path.isfile('already_prepared'):
    #        prepare_image_and_bundle(config, pod_pid)
    #        already_prepared = open('already_prepared', 'w+')
    #        already_prepared.close()
        
    set_core_states(config['disable_cores'], True)

    stress = []
    if config['stress']['run'] == "true":
        for cmd in config['stress']['commands']:
            stress.append(subprocess.Popen(cmd['args'], stdout=subprocess.DEVNULL))
            #stress.wait()

    if config['daemon']['run'] == "true":
        #run_daemon(config['daemon'])
        daemon = run_daemon_native(config['daemon'])
    
    test_loop(config)

    time.sleep(config['duration'])
    set_core_states(config['disable_cores'], False)

    if config['daemon']['run'] == "true":
        #kill_daemon()
        kill_daemon_native(daemon)
    
    if config['pod']['run'] == "true":
        kill_pod(config)
    
    if config['cgroup-version'] == 'v1':
        #set_cgroup_v2(False)
        set_cgroup_v1(False)
    
    if config['stress']['run'] == "true":
        for stressor in stress:
            stressor.kill()

    unprepare(config)
    
    
if __name__ == "__main__":
    main()