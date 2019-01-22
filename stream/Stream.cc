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
#include <stdio.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <systemd/sd-daemon.h>

#include "stream/Stream.h"
#include "shared/Frame.h"
#include "movemask/ForegroundMask.h"
#include "display/DebugDisplay.h"

// 30fps
using TimeUnit = std::chrono::duration<std::chrono::seconds::rep, std::ratio<1, 30>>;
using Clock = std::chrono::system_clock;

Stream::Stream(std::string video_src, std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals) {
    this->video_src = video_src;
    if (this->video_src.find("rtsp") == 0 || this->video_src.find("http") == 0) {
        this->cap_mode = network;
    }
    
    this->cap = std::make_shared<cv::VideoCapture>();
    this->signals = signals;
    this->fgMask = std::make_shared<ForegroundMask>();
    this->debugDisplay = std::make_shared<DebugDisplay>();

}

int Stream::start() {

    //--- INITIALIZE VIDEOCAPTURE
    // open the default camera using default API
    // std::cout<<cv::getBuildInformation()<<std::endl << std::flush;
    this->cap->open(this->video_src, cv::VideoCaptureAPIs::CAP_FFMPEG);
    std::cout << this->video_src << " CAP_PROP_FPS : " << this->cap->get(cv::VideoCaptureProperties::CAP_PROP_FPS) << std::endl << std::flush;
    
    // check if we succeeded
    if (!this->cap->isOpened()) {
        std::cout << "Unable to open video source" << std::endl << std::flush;
    }

    uint q1_size = 120;
    uint q2_size = 30;
    uint q3_size = 30;

    // Queues
    moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> queue1;
    moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> queue2;
    moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> queue3;

    this->step1 = std::thread([&](moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q) {
#ifdef USE_CUDA
        cv::cuda::Stream stream;
#endif
        int i = 0;
        double framenumber = 1;
        for (;;) {
            i++;
            std::shared_ptr<Frame> frame = std::make_shared<Frame>();
            this->cap->read(frame->cpu_raw);
            // this->write->write(frame->cpu_raw);
            // std::cout << "prop : " << this->cap->get(CAP_PROP_POS_FRAMES) << " / " << framenumber << std::endl << std::flush;
            // std::cout << "@fps : " << this->cap->get(CAP_PROP_FPS) << std::endl << std::flush;
            // if (this->d_reader->nextFrame(frame->gpu_raw)) {
            if(!frame->cpu_raw.empty()) {
                frame->frameNumber = (long)framenumber;
                framenumber++;
                if(q->size_approx() >= q1_size) {
                    // std::cout << "Pipeline stuck" << std::endl << std::flush;
                    if(this->cap_mode == local) {
                        // In local mode, we can wait for the pipe to be available
                        while(q->size_approx() >= q1_size) {
                            std::this_thread::sleep_for(TimeUnit(1)/2);
                        }
                        q->enqueue(frame);
                    }
                } else {
                    q->enqueue(frame);
                }
                // std::cout << "queue size: " << q->size_approx() << std::endl << std::flush;
            } else {
                if (!this->cap->isOpened()) {
                    std::cout << "Video source has been closed" << std::endl << std::flush;
                    if(this->cap_mode == network) {
                        std::cout << "Trying to reset network video source" << std::endl << std::flush;
                        this->cap->open(this->video_src, cv::VideoCaptureAPIs::CAP_FFMPEG);
                        std::cout << this->video_src << " CAP_PROP_FPS : " << this->cap->get(cv::VideoCaptureProperties::CAP_PROP_FPS) << std::endl << std::flush;
                    }
                }
            }
        }
    }, &queue1);

    this->step2 = std::thread([&](
        moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q1, 
        moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q2,
        std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals) {

#ifdef USE_CUDA
        cv::cuda::Stream stream;
        this->fgMask->stream = stream;
#endif
        for (;;) {
            std::shared_ptr<Frame> frame(nullptr);
            // Fully-blocking:
            q1->wait_dequeue(frame);
            // Apply gaussian blur
            cv::GaussianBlur(frame->cpu_raw, frame->cpu_smooth, cv::Size( 3, 3 ), 0, 0 );            
#ifdef USE_CUDA
            //Convert to CUDA frame
            frame->gpu_raw.upload(frame->cpu_raw, stream);
            frame->gpu_smooth.upload(frame->cpu_smooth, stream);
#endif

            // Extract moving mask
            this->fgMask->apply(frame);

#ifdef USE_CUDA
            cv::cuda::cvtColor(frame->gpu_raw, frame->gpu_gray, CV_BGR2GRAY, 0, stream);
            frame->foreground_mask.download(frame->cpu_mask, stream);
            frame->foreground_mask_raw.download(frame->cpu_mask_raw, stream);
#else
            frame->cpu_mask = frame->foreground_mask;
            frame->cpu_mask_raw = frame->foreground_mask_raw;
#endif
            while(q2->size_approx() > q2_size) {
                // std::cout << "sleep(150)" << std::endl << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            q2->enqueue(frame);
        }
    }, &queue1, &queue2, this->signals);

    this->step3 = std::thread([&](
        moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q2,
        moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q3) {
#ifdef USE_CUDA
        cv::cuda::Stream stream;
#endif
        for (;;) {
            std::shared_ptr<Frame> frame(nullptr);
            // Fully-blocking:
            q2->wait_dequeue(frame);

            // To gray
            cv::cvtColor(frame->cpu_raw, frame->cpu_gray, CV_BGR2GRAY);

            while(q3->size_approx() > q3_size) {
                // std::cout << "sleep(150)" << std::endl << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
            }
            q3->enqueue(frame);
        }
    }, &queue2, &queue3);

    this->step4 = std::thread([&](
        moodycamel::BlockingReaderWriterQueue<std::shared_ptr<Frame>> *q3) {
        std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        int frameCount = 0;
        int wait_time = 5;
        for (;;) {
            std::shared_ptr<Frame> frame(nullptr);
            // Fully-blocking:
            q3->wait_dequeue(frame);
            
            this->debugDisplay->apply(frame);

            // // std::cout << "frame " << frame->frameNumber << std::endl << std::flush;
            cv::imshow("Video", frame->cpu_display);
            // imshow("Mask", frame->cpu_mask);
            
            char key = (char)cv::waitKey(wait_time);
            if (key == 27) // Esc to exit
                 break;
            if (key == 120){ // x to switch to frame by frame
                if(wait_time == 5)
                    wait_time = 0;
                else
                    wait_time = 5;
            }
                        
            frameCount++;
            std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            std::chrono::milliseconds elapsed = now - start;
            
            if(elapsed.count() > 5000) {
                float fps = float(frameCount) / (elapsed.count() / 1000.0f);
                std::cout << "FPS at " << frame->frameNumber << " : " << fps << std::endl << std::flush;
                start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                frameCount = 0;
            }
        }
    }, &queue3);

    this->step4.join();

    return 0;
}
