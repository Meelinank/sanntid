#include "FrameReceiver.hpp"
#include <iostream>
#include <boost/asio.hpp>

FrameReceiver::FrameReceiver(boost::asio::io_service& io_service, const std::string& server, const std::string& port)
        : video_socket(io_service), receiving(false) {
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(server, port);
    boost::asio::connect(video_socket, resolver.resolve(query));
}

FrameReceiver::~FrameReceiver() {
    stopReceiving();
}

void FrameReceiver::startReceiving() {
    receiving = true;
    receiving_thread = std::thread(&FrameReceiver::receiveFrames, this);
}

void FrameReceiver::stopReceiving() {
    receiving = false;
    if (receiving_thread.joinable()) {
        receiving_thread.join();
    }
}

bool FrameReceiver::getNextFrame(cv::Mat& frame) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (!frame_queue.empty()) {
        frame = frame_queue.front();
        frame_queue.pop();
        return true;
    }
    return false;
}

bool FrameReceiver::hasFrames() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return !frame_queue.empty();
}

void FrameReceiver::receiveFrames() {
    try {
        while (receiving) {
            unsigned int length;
            boost::asio::read(video_socket, boost::asio::buffer(&length, sizeof(length)));
            length = ntohl(length);

            if (length > 10000000) {
                std::cerr << "Frame size too large, possible misalignment\n";
                continue;
            }

            std::vector<uchar> buffer(length);
            boost::asio::read(video_socket, boost::asio::buffer(buffer.data(), buffer.size()));

            decodeFrame(buffer);
        }
    } catch (std::exception& e) {
        std::cerr << "Error in frame reception: " << e.what() << std::endl;
        receiving = false;
    }
}

void FrameReceiver::decodeFrame(const std::vector<uchar>& buffer) {
    cv::Mat frame = cv::imdecode(buffer, cv::IMREAD_COLOR);
    if (!frame.empty()) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        frame_queue.push(frame);
    }
}