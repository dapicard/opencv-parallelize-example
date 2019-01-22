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


new_local_repository(
    name = "opencv",
    path = "/usr/local",
    build_file = "opencv.BUILD",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")

new_git_repository(
    name = "rwqueue",
    build_file = "rwqueue.BUILD",
    remote = "https://github.com/cameron314/readerwriterqueue.git",
    commit = "07e22ecdf90501df89ead679bb8294a0b7c80c24"
)