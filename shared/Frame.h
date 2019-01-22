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
#include <opencv2/opencv.hpp>

class Frame {
public:
    cv::Mat cpu_raw;
    cv::Mat cpu_smooth;
    cv::Mat cpu_gray;
    cv::Mat cpu_mask;
    cv::Mat cpu_mask_raw;
    cv::Mat cpu_display;
    cv::Mat cpu_oldGray;

#ifdef USE_CUDA
    cv::cuda::GpuMat gpu_raw;
    cv::cuda::GpuMat gpu_smooth;
    cv::cuda::GpuMat gpu_gray;
    cv::cuda::GpuMat foreground_mask;
    cv::cuda::GpuMat foreground_mask_raw;
    cv::cuda::GpuMat gpu_oldGray;
#else
    cv::Mat foreground_mask;
    cv::Mat foreground_mask_raw;
#endif
    long frameNumber;

    int width();
    int height();
};