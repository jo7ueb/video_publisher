#include <iostream>
#include <string>
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <boost/program_options.hpp>

#define QD 10

int main(int argc, char **argv)
{
    std::string topic_name = "/camera/image_raw";
    std::string file_name  = "0016E5.MXF";
    bool from_video = false;
    int video = 0;
    double speed = 1.0;

    // parse command line
    namespace po = boost::program_options;
    po::options_description opt("option");
    opt.add_options()
        ("help,h", "display help")
        ("topic,t", po::value<std::string>(&topic_name), "output topic name")
        ("input,i", po::value<std::string>(&file_name), "input video file path")
        ("video,v", po::value<int>(&video), "video device id")
        ("speed,s", po::value<double>(&speed), "playback speed");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, opt), vm);
        po::notify(vm);
    }
    catch(boost::program_options::error& e) {
        std::cerr << "Command error: " << e.what() << std::endl;
        return -1;
    }

    if(vm.count("help")) {
        std::cout << opt << std::endl;
        return 0;
    }
    if(vm.count("video")) {
        from_video = true;
    }

    std::cout << "Topic name: " << topic_name << std::endl;
    if(from_video) {
        std::cout << "Capture device: " << video << std::endl;
    } else {
        std::cout << "Video path: " << file_name << std::endl;
    }
    std::cout << "Playback speed: " << speed << std::endl;

    // ROS settings
    ros::init(argc, argv, "publisher");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise(topic_name, QD);

    // open video file
    cv::VideoCapture cap;
    if(from_video) {
        cap.open(video);
    } else {
        cap.open(file_name);
    }
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
    ros::Rate rate(fps * speed);
    size_t frameIdx = 0;
    while(ros::ok()) {
        cv::Mat frame;
        cap >> frame;

        const double ratio = frameIdx / length;
        std::cout << "frame#" << frameIdx++ << " (" << ratio*100 << "%)\r" << std::flush;

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
