import cv2
import asyncio
import websockets

async def send_video():
    # Open the default camera
    cap = cv2.VideoCapture(0)

    # Set the video size to 640x480
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Connect to the server
    server_address = ("192.168.199.175", 12345)

    # Start the video stream
    while True:
        # Capture a frame from the camera
        ret, frame = cap.read()

        # Convert the frame to a string
        frame_str = cv2.imencode('.jpg', frame)[1].tostring()

        # Send the frame over UDP
        sock.sendto(frame_str, server_address)

        # Wait for a short time
        await asyncio.sleep(0.01)

    # Release the camera and close the socket
    cap.release()
    sock.close()

async def main():
    async with websockets.connect('ws://localhost:8765') as websocket:
        await websocket.send('start')
        await send_video()

asyncio.run(main())
"""
// FILEPATH: /path/to/receive_video.cpp
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345

int main() {
    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Bind the socket to a local address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }

    // Create a window to display the video
    cv::namedWindow("Video", cv::WINDOW_NORMAL);

    // Receive and display the video stream
    while (true) {
        // Receive a frame from the socket
        char buffer[640*480*3];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
        if (n < 0) {
            std::cerr << "Failed to receive frame" << std::endl;
            break;
        }

        // Convert the frame to an OpenCV image
        cv::Mat frame = cv::imdecode(cv::Mat(1, n, CV_8UC1, buffer), cv::IMREAD_COLOR);

        // Display the frame
        cv::imshow("Video", frame);
        cv::waitKey(1);
    }

    // Close the socket and destroy the window
    close(sockfd);
    cv::destroyAllWindows();

    return 0;
}
"""