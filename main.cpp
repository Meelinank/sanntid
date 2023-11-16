#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <SDL2/SDL.h>
#include <opencv2/opencv.hpp>

using boost::asio::ip::tcp;

tcp::socket* global_socket = nullptr;
std::mutex image_mutex;
cv::Mat global_frame;
bool new_frame_available = false;

void send_command(tcp::socket& socket, const std::string& command) {
    try {
        std::cout << "Sending command: " << command << std::endl;
        boost::asio::write(socket, boost::asio::buffer(command + "\n"));
    } catch (const std::exception& e) {
        std::cerr << "Error sending command: " << e.what() << std::endl;
    }
}

void videoStreamThread() {
    try {
        while (true) {
            boost::asio::streambuf b;
            boost::asio::read(*global_socket, b, boost::asio::transfer_exactly(4));
            std::istream is(&b);
            uint32_t image_len;
            is.read(reinterpret_cast<char *>(&image_len), 4);

            if (image_len == 0) break;

            std::vector<uchar> buffer(image_len);
            boost::asio::read(*global_socket, boost::asio::buffer(buffer.data(), image_len));

            cv::Mat frame = cv::imdecode(cv::Mat(buffer), cv::IMREAD_COLOR);
            if (frame.empty()) {
                std::cerr << "Received an empty frame!" << std::endl;
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(image_mutex);
                global_frame = frame.clone();
                new_frame_available = true;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Video Stream Thread Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        global_socket = &socket;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("192.168.1.166", "8050");
        boost::asio::connect(socket, endpoints);

        std::thread videoThread(videoStreamThread);

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return 1;
        }

        std::cout << "Connected to the server. Press arrow keys to control the robot. Press 'q' to quit." << std::endl;

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                } else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP: send_command(socket, "FORWARD"); break;
                        case SDLK_DOWN: send_command(socket, "BACKWARD"); break;
                        case SDLK_LEFT: send_command(socket, "LEFT"); break;
                        case SDLK_RIGHT: send_command(socket, "RIGHT"); break;
                        case SDLK_s: send_command(socket, "STOP"); break;
                        case SDLK_q: quit = true; break;
                    }
                }
            }

            {
                std::lock_guard<std::mutex> lock(image_mutex);
                if (new_frame_available) {
                    cv::imshow("Video", global_frame);
                    new_frame_available = false;
                }
            }

            if (cv::waitKey(1) == 'q') {
                break;
            }
        }

        videoThread.join();
        SDL_Quit();
    } catch (const std::exception& e) {
        std::cerr << "Main Exception: " << e.what() << std::endl;
        SDL_Quit();
    }

    return 0;
}
