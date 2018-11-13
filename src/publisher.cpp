#include <iostream>
#include <string>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#define QD 10

int main(int argc, char **argv)
{
    std::string topic_name = "/camera/image_raw";
    std::string file_name  = "0016E5.MXF";

    // ROS settings
    ros::init(argc, argv, "publisher");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise(topic_name, QD);

    // open video file
    cv::VideoCapture cap(file_name);
    if(!cap.isOpened()) {
        std::cerr << "Failed to open " << file_name << std::endl;
        return -1;
    }
    const double w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    const double h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    const double fps = cap.get(CV_CAP_PROP_FPS);
    const double length = cap.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout << file_name << "(" << w << "x" << h
              << " " << fps << "FPS) opened." << std::endl;

    // publish each video frame
    ros::Rate rate(fps);
    size_t frameIdx = 0;
    while(ros::ok()) {
        cv::Mat frame;
        cap >> frame;

        const double ratio = frameIdx / length;
        std::cout << "frame#" << frameIdx++ << " (" << ratio*100 << "%)\r";

        if (frame.empty()) {
            std::cout << "reach to the end" << std::endl;
            break;
        }

        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame).toImageMsg();
        pub.publish(msg);

        ros::spinOnce();
        rate.sleep();
    }
}
