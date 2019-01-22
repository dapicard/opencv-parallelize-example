# opencv-parallelize-example

This project is a pattern project showing how I implement parallel computing from a video stream using :
 - OpenCV (3.4.5) with or without CUDA
 - MoodyCamel's readerwriterqueue https://github.com/cameron314/readerwriterqueue
Also, this project shows how to notify SystemD if it is used as a service, and the executable can be notified using signals (USR1 and USR2 currently)

This is built with Bazel :
 - OpenCV should be installed in /usr/local
 - Systemd headers (-dev package) have to be installed

The example show how to extract Foreground (ForegroundMask), do some color conversions, etc.
Of course, you can add every computing you want...

Create issues if you have any questions.