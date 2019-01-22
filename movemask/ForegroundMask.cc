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
#include "shared/Frame.h"
#include "ForegroundMask.h"

ForegroundMask::ForegroundMask() {

#ifdef USE_CUDA
    this->pMOG2 = cv::cuda::createBackgroundSubtractorMOG2(5000, 32, true);
#else
    this->pMOG2 = cv::createBackgroundSubtractorMOG2(5000, 32, true);
#endif
    pMOG2->setShadowThreshold(0.3);
    pMOG2->setShadowValue(0);
}

void ForegroundMask::apply(std::shared_ptr<Frame> frame) {
#ifdef USE_CUDA
    this->pMOG2->apply(frame->gpu_smooth, frame->foreground_mask, -1.0, this->stream);
#else
    this->pMOG2->apply(frame->cpu_smooth, frame->foreground_mask);
#endif
}