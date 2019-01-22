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

#pragma once
#include <thread>
#include <memory>
#include <opencv2/opencv.hpp>
#include "readerwriterqueue.h"

#include "shared/Frame.h"
#include "movemask/ForegroundMask.h"
#include "display/DebugDisplay.h"

enum CaptureMode { local, network };

class Stream {
private:
public:
    std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals;
    std::string video_src;
    CaptureMode cap_mode = local;
    std::shared_ptr<Frame> current_frame;
    std::shared_ptr<cv::VideoCapture> cap;

    std::shared_ptr<ForegroundMask> fgMask;
    std::shared_ptr<DebugDisplay> debugDisplay;

    std::thread step1;
    std::thread step2;
    std::thread step3;
    std::thread step4;

    Stream(std::string video_src, std::shared_ptr<moodycamel::ReaderWriterQueue<int>> signals);
    int start();
};