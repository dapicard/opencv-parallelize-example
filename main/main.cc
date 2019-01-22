// This file is part of opencv-parallelize-example.

// opencv-parallelize-example is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// opencv-parallelize-example is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with opencv-parallelize-example.  If not, see <https://www.gnu.org/licenses/>.

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <systemd/sd-daemon.h>
#include <memory>
#include "readerwriterqueue.h"
#include "stream/Stream.h"

std::vector<std::shared_ptr<moodycamel::ReaderWriterQueue<int>>> signal_queues;

// To send signal with systemd : 
// systemctl kill -s SIGUSR1 camex
void signal_handler( int signum ) {
   std::cout << "Receiving signal " << signum << std::endl << std::flush;
   for(std::shared_ptr<moodycamel::ReaderWriterQueue<int>> &signals : signal_queues) {
       signals->enqueue(signum);
   }
}

void sigv_handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char* argv[])
{
    
    std::cout << "Register signals handler" << std::endl << std::flush;
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGSEGV, sigv_handler);

    std::string video_url;
    if(argc > 1) {
        video_url = argv[1];
    } else {
        video_url = "/home/damien/Vidéos/cam1-2018.11.22-16.21.04.avi";
    }
    
    // vector useful when handling multiple streams
    std::vector<std::thread> threads;
    // for(every source) {
    // Queue used to broadcast signals to underlying threads
    std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals = std::make_shared<moodycamel::ReaderWriterQueue<int>>(1);
    signal_queues.push_back(signals);
    threads.push_back(std::thread([&](std::string video_url, std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals) {
        
        std::cout << "Input : " << video_url << std::endl << std::flush;
        Stream stream = Stream(video_url, signals);
        stream.current_frame = std::make_shared<Frame>();
        stream.start();
    }, video_url, signals));
    //}
    
    std::cout << "Stream Ready" << std::endl << std::flush;
    sd_notify(0, "READY=1");
    
    for(std::thread &t : threads) {
        t.join();
    }
    
    return 0;
}