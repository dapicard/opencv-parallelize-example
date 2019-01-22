#    This file is part of opencv-parallelize-example.
#
#    opencv-parallelize-example is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    opencv-parallelize-example is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with opencv-parallelize-example.  If not, see <https://www.gnu.org/licenses/>.


cc_binary(
    name = "parallelize",
    srcs = ["main.cc"],
    defines = ["USE_CUDA"],
    deps = [
        "@opencv//:opencv",
        "//stream:stream",
    ],
    linkopts = [
        "-pthread",
        "-lsystemd",
        "-rdynamic"
    ]
)