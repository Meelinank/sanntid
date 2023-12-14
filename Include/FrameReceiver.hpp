#ifndef SANNTID_FRAMERECEIVER_HPP
#define SANNTID_FRAMERECEIVER_HPP

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

class FrameReceiver {
public:
    FrameReceiver(boost::asio::io_service& io_service, const std::string& server, const std::string& port);
    ~FrameReceiver();

    void startReceiving();
    void stopReceiving();
    bool getNextFrame(cv::Mat& frame);
    bool hasFrames() const;

private:
    void receiveFrames();
    void decodeFrame(const std::vector<uchar>& buffer);

    boost::asio::ip::tcp::socket video_socket;
    std::queue<cv::Mat> frame_queue;
    mutable std::mutex queue_mutex;
    std::atomic<bool> receiving;
    std::thread receiving_thread;
};
#endif //SANNTID_FRAMERECEIVER_HPP