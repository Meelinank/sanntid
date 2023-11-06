#include <iostream>
#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>

using boost::asio::ip::tcp;

int main() {

    // Connect to the server
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("10.25.45.112"), 8000));

    try {
        while (true) {
            boost::asio::streambuf b;
            boost::asio::read(socket, b, boost::asio::transfer_exactly(4));  // Read the size of the image
            std::istream is(&b);
            uint32_t image_len;
            is.read(reinterpret_cast<char *>(&image_len), 4);

            // Allocate a buffer and read the image data
            std::vector<uchar> buffer(image_len);
            boost::asio::read(socket, boost::asio::buffer(buffer.data(), image_len));

            // Decode the image
            cv::Mat frame = imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
            if (frame.empty()) {
                std::cerr << "Received an empty frame!" << std::endl;
                break;
            }

            // Display the frame
            imshow("Video", frame);
            if (cv::waitKey(1) == 'q') {
                break;
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Close the connection
    socket.close();

    return 0;
}