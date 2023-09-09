// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <webcface/client.h>
#include <webcface/data.h>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher() : Node("minimal_publisher"), wcli_("b", "172.17.0.1")
    {
        std::thread([&] {
            for (int i = 0; i < 100; i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                wcli_.sync();
            }
        }).detach();
        // コンストラクタが終了する前にpublisherつくらないといけないっぽいので適当に通信完了を待つ
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (auto m : wcli_.members()) {
            // std::cout << "memberentry" << std::endl;
            for (auto v : m.values()) {
                // std::cout << "valueentry" << std::endl;
                auto topic_name = m.name() + "/" + v.name();
                for (std::size_t i = 0; i < topic_name.size(); i++) {
                    if (topic_name[i] == '.') {
                        topic_name[i] = '/';
                    }
                }
                auto pub = this->create_publisher<std_msgs::msg::Float64>(topic_name, 10);
                publisher_.push_back(pub);
                auto publish = [&, pub, topic_name](auto v) {
                    auto message = std_msgs::msg::Float64();
                    message.data = v.get();
                    RCLCPP_INFO(this->get_logger(), "Publishing: %s = '%f'", topic_name.c_str(),
                        message.data);
                    pub->publish(message);
                };
                publish(v);
                v.appendListener(publish);
            }
        }
        // std::cout << "constructor end" << std::endl;
    }

private:
    std::vector<rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr> publisher_;

    WebCFace::Client wcli_;
};

int main(int argc, char* argv[])
{
    WebCFace::logger_internal_level = spdlog::level::debug;

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}
