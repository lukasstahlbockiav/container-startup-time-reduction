// Copyright (C) 2014-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Updated by Lukas Stahlbock in 2022
// Copyright (c) 2022 IAV GmbH Ingenieurgesellschaft Auto und Verkehr. All rights reserved.
// Initial code from https://github.com/COVESA/vsomeip/blob/3.1.16.1/examples/subscribe-sample.cpp
//
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
#include <csignal>
#endif
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <sstream>
#include <thread>
#include <mutex>

#include <vsomeip/vsomeip.hpp>
#include <vsomeip/internal/logger.hpp>

#include "service_ids.hpp"
//includes for change_mode
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_FIFO_BUFFER 1

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    static bool stop_application = false;
    static bool stop_sighandler = false;
    static std::condition_variable_any sighandler_condition;
    static std::recursive_mutex sighandler_mutex;
#endif

class service_sample {
public:
    service_sample(bool _use_tcp) :
            app_(vsomeip::runtime::get()->create_application()),
            is_registered_(false),
            use_tcp_(_use_tcp),
            use_someip_tp_(false) {
    }

    bool init_request_event(vsomeip::service_t _service_id, vsomeip::instance_t _service_instance_id, vsomeip::eventgroup_t _service_eventgroup_id, vsomeip::event_t _service_event_id){
        app_->register_message_handler(
                _service_id, _service_instance_id, _service_event_id,
                std::bind(&service_sample::on_message, this,
                        std::placeholders::_1));

        app_->register_availability_handler(_service_id, _service_instance_id,
                std::bind(&service_sample::on_availability,
                          this,
                          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), major_version, minor_version);

        std::set<vsomeip::eventgroup_t> its_groups;
        its_groups.insert(_service_eventgroup_id);
        app_->request_event(
                _service_id,
                _service_instance_id,
                _service_event_id,
                its_groups,
                vsomeip::event_type_e::ET_EVENT,
                vsomeip_v3::reliability_type_e::RT_UNRELIABLE);
        //app_->subscribe(_service_id, _service_instance_id, _service_eventgroup_id, major_version);
        //std::cout << "Subscribed to service " << _service_id << ":" << _service_instance_id << ":" << _service_eventgroup_id << ":" << unsigned(major_version) << std::endl;
        return true;
    }

    bool init() {
        std::lock_guard<std::mutex> its_lock(mutex_);

        if (!app_->init()) {
            std::cerr << "Couldn't initialize application" << std::endl;
            return false;
        }
        app_->register_state_handler(
                std::bind(&service_sample::on_state, this,
                        std::placeholders::_1));

        char *env_var = getenv("VSOMEIP_CLIENT");
        std::stringstream its_converter;
        bool ret;
        if (nullptr != env_var) {
            for (int i = 0; i < std::strlen(env_var); i++){
                //std::cout << "Checking " << env_var[i] << " of " << env_var << std::endl;
                if (env_var[i] == '1'){
                    service_id_client = service_id_list[i];
                    //std::cout << "client id " << service_id_client << " with i: " << i << std::endl;
                    service_eventgroup_id_client = service_id_client;
                    ret = init_request_event(service_id_client, service_instance_id, service_eventgroup_id_client, service_event_id);
                    if (ret == false){
                        VSOMEIP_ERROR << "Could not subscribe to service " << service_id_client;
                    }
                }
            }
        }

        env_var = getenv("VSOMEIP_SERVER");
        if (nullptr != env_var) {
            //std::cout << "env_var: " << env_var << std::endl;
            if (env_var[0] == '0' && env_var[1] == 'x') {
                its_converter << std::hex << env_var;
            } else {
                its_converter << std::dec << env_var;
            }
            its_converter >> service_id_server;
            service_eventgroup_id_server = service_id_server;
            //std::cout << "Server ID " << service_id_server << std::endl;
            its_converter.str("");
            its_converter.clear();

            env_var = getenv("VSOMEIP_SERVER_CYCLE");
            //std::cout << "env_var: " << env_var << std::endl;
            its_converter << std::dec << env_var;
            its_converter >> cycle_;
            //std::cout << "Cycle " << cycle_ << std::endl;
            its_converter.str("");
            its_converter.clear();

            env_var = getenv("VSOMEIP_SERVER_DATA_SIZE");
            //std::cout << "env_var: " << env_var << std::endl;
            its_converter << std::dec << env_var;
            its_converter >> data_size_; 
            //std::cout << "Data size " << data_size_ << std::endl;
            its_converter.str("");
            its_converter.clear();

            if (data_size_ > VSOMEIP_MAX_UDP_MESSAGE_SIZE - 160){
                use_someip_tp_ = true;
            }

            operation_mode_ = 0;
            env_var = getenv("VSOMEIP_CHANGE_MODE");
            if (nullptr != env_var) {
                mkfifo(change_mode_fifo_, 0666);
                fifo_thread_ = std::thread(&service_sample::listen_fifo, this);
            }
            env_var = getenv("VSOMEIP_CHANGE_MODE_CYCLE");
            if (nullptr != env_var) {
                its_converter << std::dec << env_var;
                its_converter >> operation_mode_cycle_;
                its_converter.str("");
                its_converter.clear();
            } else {
                operation_mode_cycle_ = 10;
            }

            running_ = true;
            blocked_ = false;
            is_offered_ = false;
            offer_thread_ = std::thread(&service_sample::run, this);
            notify_thread_ = std::thread(&service_sample::notify, this);

            std::set<vsomeip::eventgroup_t> its_groups;
            its_groups.insert(service_eventgroup_id_server);
            app_->offer_event(
                    service_id_server,
                    service_instance_id ,
                    service_event_id,
                    its_groups,
                    vsomeip::event_type_e::ET_EVENT, std::chrono::milliseconds::zero(),
                    false, true, nullptr, 
                    (use_tcp_ ? vsomeip::reliability_type_e::RT_RELIABLE : vsomeip::reliability_type_e::RT_UNRELIABLE));
            {
                std::lock_guard<std::mutex> its_lock(payload_mutex_);
                payload_ = vsomeip::runtime::get()->create_payload();
            }

            blocked_ = true;
            condition_.notify_one();
            env_var = getenv("VSOMEIP_CHANGE_MODE");
            if (nullptr != env_var) {
                fifo_condition_.notify_one();
            }
        }

        stress_cpu_ = false;
        env_var = getenv("VSOMEIP_STRESS_CPU");
        if (nullptr != env_var) {
            stress_cpu_ = true;
            stress_cpu_iterations_ = 5000;

            env_var = getenv("VSOMEIP_STRESS_CPU_ITERATIONS");
            if (nullptr != env_var) {
                its_converter << std::dec << env_var;
                its_converter >> stress_cpu_iterations_;
                its_converter.str("");
                its_converter.clear();
                //std::cout << "stress cpu iterations is: " << stress_cpu_iterations_ << std::endl;
            }
            //stress_cpu_thread_ = std::thread(&service_sample::stress_cpu, this);
        }

        return true;
    }

    void start() {
        app_->start();
    }

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    /*
     * Handle signal to shutdown
     */
    void stop() {
        VSOMEIP_INFO << "stop - release services";
        char *env_var = getenv("VSOMEIP_CLIENT");
        if (nullptr != env_var) {
            for (int i = 0; i < std::strlen(env_var); i++){
                if (env_var[i] == '1'){
                    service_id_client = service_id_list[i];
                    service_eventgroup_id_client = service_id_client;
                    app_->unsubscribe(service_id_client, service_instance_id, service_eventgroup_id_client, service_event_id);
                    app_->release_event(service_id_client, service_instance_id, service_event_id);
                    app_->release_service(service_id_client, service_instance_id);
                    app_->unregister_message_handler(service_id_client, service_instance_id, service_event_id);
                    app_->unregister_availability_handler(service_id_client, service_instance_id, major_version, minor_version);
                }
            }
        }
        VSOMEIP_INFO << "stop - stopping server";
        env_var = getenv("VSOMEIP_SERVER");
        if (nullptr != env_var) {
            running_ = false;
            blocked_ = true;
            condition_.notify_one();
            notify_condition_.notify_one();
            stop_offer();
            offer_thread_.join();
            notify_thread_.join();
            /*
            env_var = getenv("VSOMEIP_CHANGE_MODE");
            if (nullptr != env_var) {
                fifo_condition_.notify_one();
                fifo_thread_.join();
            }
            */
        }
        VSOMEIP_INFO << "stop - disable cpu stress";
        stress_cpu_ = false;
        VSOMEIP_INFO << "stop - clear_all_handler";
        app_->clear_all_handler();
        VSOMEIP_INFO << "stop - stop";
        app_->stop();
    }
#endif

    void offer() {
        std::lock_guard<std::mutex> its_lock(notify_mutex_);
        app_->offer_service(service_id_server, service_instance_id, major_version, minor_version );
        is_offered_ = true;
        notify_condition_.notify_one();
    }

    void stop_offer() {
        app_->stop_offer_service(service_id_server, service_instance_id, major_version, minor_version );
        is_offered_ = false;
    }

    void on_state(vsomeip::state_type_e _state) {
        VSOMEIP_DEBUG << "Application " << app_->get_name() << " is "
        << (_state == vsomeip::state_type_e::ST_REGISTERED ?
                "registered." : "deregistered.");

        if (_state == vsomeip::state_type_e::ST_REGISTERED) {
            if (!is_registered_) {
                is_registered_ = true;
                char *env_var = getenv("VSOMEIP_CLIENT");
                if (nullptr != env_var) {
                    for (int i = 0; i < std::strlen(env_var); i++){
                        if (env_var[i] == '1'){
                            app_->request_service(service_id_list[i], service_instance_id, major_version, minor_version);
                        }
                    }
                }
            }
        } else {
            is_registered_ = false;
        }
    }

    void on_availability(vsomeip::service_t _service, vsomeip::instance_t _instance, bool _is_available) {
        VSOMEIP_DEBUG << "Service ["
                << std::setw(4) << std::setfill('0') << std::hex << _service << "." << _instance
                << "] is "
                << (_is_available ? "available." : "NOT available.");
        if (_is_available){
            char *env_var = getenv("VSOMEIP_CLIENT");
            if (nullptr != env_var) {
                for (int i = 0; i < std::strlen(env_var); i++){
                    if (env_var[i] == '1'){
                        if (service_id_list[i] == _service){
                            app_->subscribe(_service, _instance, _service, major_version);
                        }
                    }
                }
            }
        }
    }

    void on_message(const std::shared_ptr<vsomeip::message> &_message) {
        if (stress_cpu_){
            stress_cpu();
        }
        
        std::stringstream its_message;
        its_message << "Received a notification for Event ["
                << std::setw(4)    << std::setfill('0') << std::hex
                << _message->get_service() << "."
                << std::setw(4) << std::setfill('0') << std::hex
                << _message->get_instance() << "."
                << std::setw(4) << std::setfill('0') << std::hex
                << _message->get_method() << "] to Client/Session ["
                << std::setw(4) << std::setfill('0') << std::hex
                << _message->get_client() << "/"
                << std::setw(4) << std::setfill('0') << std::hex
                << _message->get_session()
                << "] with length "
                << _message->get_length();
        //VSOMEIP_INFO << its_message.str();
        
        
        std::shared_ptr<vsomeip::payload> its_payload =
                _message->get_payload();
        its_message << "(" << std::dec << its_payload->get_length() << ") ";
        
        // print total payload
        /*
        for (uint32_t i = 0; i < its_payload->get_length(); ++i)
            its_message << std::hex << std::setw(2) << std::setfill('0')
                << (int) its_payload->get_data()[i] << " ";
        */
       // print only first 4 bytes
       for (uint32_t i = 0; i < 4; ++i)
            its_message << std::hex << std::setw(2) << std::setfill('0')
                << (int) its_payload->get_data()[i] << " ";
        VSOMEIP_DEBUG << its_message.str();
    }

    void run() {
        std::unique_lock<std::mutex> its_lock(mutex_);
        while (!blocked_)
            condition_.wait(its_lock);

        bool is_offer(true);
        while (running_) {
            if (is_offer)
                offer();
        }
    }

    std::vector<vsomeip::byte_t> generate_payload(std::uint32_t _number_of_fragments,
            std::uint32_t _segment_size) {
        std::vector<vsomeip::byte_t> its_data;
        for (std::uint32_t i = 0; i < _number_of_fragments; i++) {
            its_data.resize((i * _segment_size) + _segment_size,
                    static_cast<std::uint8_t>(0));
        }
        return its_data;
    }

    void notify() {    
        std::unique_lock<std::mutex> its_lock(notify_mutex_);
        while (!blocked_)
            notify_condition_.wait(its_lock);
        
        vsomeip::byte_t its_data[data_size_];
        std::vector<vsomeip::byte_t> its_data_vector;

        if (!use_someip_tp_){
            for (uint32_t i = 0; i < data_size_; ++i)
                its_data[i] = static_cast<uint8_t>(0);

            VSOMEIP_INFO << "Size of payload: " << sizeof(its_data);
        } else {
            uint16_t number_of_fragments = data_size_ / VSOMEIP_MAX_UDP_MESSAGE_SIZE + 1;
            its_data_vector = generate_payload(number_of_fragments,
                VSOMEIP_MAX_UDP_MESSAGE_SIZE - 160);
        }
                
        uint32_t its_ctr = 1;

        while (running_) {
            
            while (!is_offered_ && running_)
                notify_condition_.wait(its_lock);
            while (is_offered_ && running_) {
                if (its_ctr == 0xffffffff)
                    its_ctr = 1;

                if (!use_someip_tp_){
                    its_data[0] = (its_ctr & 0x000000ff);
                    its_data[1] = (its_ctr & 0x0000ff00) >> 8;
                    its_data[2] = (its_ctr & 0x00ff0000) >> 16;
                    its_data[3] = (its_ctr & 0xff000000) >> 24;
                    std::lock_guard<std::mutex> its_lock(payload_mutex_);
                    payload_->set_data(its_data, data_size_);
                } else {
                    std::lock_guard<std::mutex> its_lock(payload_mutex_);
                    payload_->set_data(its_data_vector);
                }
                VSOMEIP_DEBUG << "Setting event (Length=" << std::dec << data_size_ << ").";
                app_->notify(service_id_server, service_instance_id , service_event_id, payload_);
                
                if (operation_mode_ != 1){
                    its_ctr++;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(cycle_));
            }
        }
    }

    unsigned long long factorial(int n){
        unsigned long long f;
        for(int i = 1; i <= n; ++i){
            f *= i;
        }
        return f;
    }

    void stress_cpu() {
        for (int i = 0; i < stress_cpu_iterations_; i++){
            volatile int n = 30;
            volatile unsigned long long result = factorial(n);
        }

    }

    void listen_fifo() {
        std::unique_lock<std::mutex> its_lock(fifo_mutex_);
        while (!blocked_)
            fifo_condition_.wait(its_lock);

        int fd;
        char buffer[1];
        ssize_t size_read_;
        uint16_t cycle_old_ = 0;
        std::stringstream its_converter;
        VSOMEIP_DEBUG << "Start reading fifo";
        while (running_) {
            fd = open(change_mode_fifo_, O_RDONLY);
            if (fd == -1){
                VSOMEIP_ERROR << "open pipe";
            }
            size_read_ = read(fd, buffer, MAX_FIFO_BUFFER);
            if (size_read_ >= 1){
                its_converter << buffer;
                its_converter >> operation_mode_;
                its_converter.str("");
                its_converter.clear();
                VSOMEIP_DEBUG << "Received operation mode: " << operation_mode_;
                //operation_mode_ = atoi(buffer);
                if (operation_mode_ == 1) {
                    cycle_old_ = cycle_;
                    cycle_ = operation_mode_cycle_;
                    VSOMEIP_DEBUG << "Changed cycle time to " << cycle_;
                } else if (cycle_old_ != 0) {
                    cycle_ = cycle_old_;
                    VSOMEIP_DEBUG << "Changed cycle time to " << cycle_old_;
                }
            } else if (size_read_ == 0){
                VSOMEIP_DEBUG << "EOF pipe";
            } else if (size_read_ == -1) {
                VSOMEIP_ERROR << "read pipe";
            }
            
            close(fd);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
private:
    std::shared_ptr<vsomeip::application> app_;
    bool is_registered_;
    bool use_tcp_;
    uint16_t data_size_;
    uint16_t cycle_;
    bool use_someip_tp_;

    std::mutex mutex_;
    std::condition_variable condition_;
    bool blocked_;
    bool running_;

    std::mutex notify_mutex_;
    std::condition_variable notify_condition_;
    bool is_offered_;

    std::mutex payload_mutex_;
    std::shared_ptr<vsomeip::payload> payload_;

    // blocked_ / is_offered_ must be initialized before starting the threads!
    std::thread offer_thread_;
    std::thread notify_thread_;

    // stressors
    bool stress_cpu_;
    uint16_t stress_cpu_iterations_;

    bool stress_mem_;
    std::thread stress_mem_thread_;

    // change_mode 
    const char* change_mode_fifo_ = "/tmp/change_mode_fifo";
    std::mutex fifo_mutex_;
    std::condition_variable fifo_condition_;
    std::thread fifo_thread_;
    uint8_t operation_mode_;
    uint16_t operation_mode_cycle_;
};

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    service_sample *its_sample_ptr(nullptr);
    void handle_signal(int _signal) {
        if (its_sample_ptr != nullptr && (_signal == SIGINT || _signal == SIGTERM)) {
            stop_application = true;
            //its_sample_ptr->stop();
        }
        std::unique_lock<std::recursive_mutex> its_lock(sighandler_mutex);
        sighandler_condition.notify_one();
    }
    
#endif

int main(int argc, char **argv) {
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    // Block all signals
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);
#endif
    bool use_tcp = false;

    std::string tcp_enable("--tcp");
    std::string udp_enable("--udp");

    for (int i = 1; i < argc; i++) {
        if (tcp_enable == argv[i]) {
            use_tcp = true;
            break;
        }
        if (udp_enable == argv[i]) {
            use_tcp = false;
            break;
        }

    }

    service_sample its_sample(use_tcp);
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    its_sample_ptr = &its_sample;
    std::thread sighandler_thread([]() {
        // Unblock signals for this thread only
        sigset_t handler_mask;
        sigemptyset(&handler_mask);
        sigaddset(&handler_mask, SIGTERM);
        sigaddset(&handler_mask, SIGINT);
        sigaddset(&handler_mask, SIGSEGV);
        sigaddset(&handler_mask, SIGABRT);
        pthread_sigmask(SIG_UNBLOCK, &handler_mask, NULL);

        // Handle the following signals
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        while (!stop_sighandler) {
            std::unique_lock<std::recursive_mutex> its_lock(sighandler_mutex);
            sighandler_condition.wait(its_lock);
            if (stop_application) {
                its_sample_ptr->stop();
                return;
            }
        }
    });
#endif
/*
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    its_sample_ptr = &its_sample;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
#endif
*/
    if (its_sample.init()) {
        its_sample.start();
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
        sighandler_thread.join();
#endif
        return 0;
    }
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    std::unique_lock<std::recursive_mutex> its_lock(sighandler_mutex);
    stop_sighandler = true;
    sighandler_condition.notify_one();
    sighandler_thread.join();
#endif
    return 1;
}