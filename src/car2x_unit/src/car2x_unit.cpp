#include "rclcpp/rclcpp.hpp"
#include "cpm_interface.pb.h"
#include <boost/asio.hpp>
#include "std_msgs/msg/string.hpp"

using boost::asio::ip::udp;
using namespace std;

class UDPListenerNode : public rclcpp::Node {
public:
    UDPListenerNode() : Node("udp_listener") {
        publisher_ = this->create_publisher<std_msgs::msg::String>("udp_data", 10);

        socket_ = std::make_unique<udp::socket>(io_context_, udp::endpoint(udp::v4(), 12345));

        receive();
    }

private:
    void receive() {
        const int max_length = 1024;
        char buffer[max_length];

        udp::endpoint sender_endpoint;
        size_t length = socket_->receive_from(boost::asio::buffer(buffer, max_length), sender_endpoint);

        CPMMessage message;
        if (!message.ParseFromArray(buffer, length)) {
            RCLCPP_ERROR(this->get_logger(), "Failed to parse received data");
            return;
        }

        auto str_msg = std_msgs::msg::String();
        str_msg.data = message.DebugString();
        publisher_->publish(str_msg);

        RCLCPP_INFO(this->get_logger(), "Received message: %s\n-----------------\n", message.DebugString().c_str());

        receive();
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    boost::asio::io_context io_context_;
    std::unique_ptr<udp::socket> socket_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<UDPListenerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
