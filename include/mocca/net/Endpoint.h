#pragma once

#include <string>

namespace mocca {
namespace net {

struct Endpoint {
public:
    Endpoint();
    Endpoint(const std::string& protocol, const std::string& machine, const std::string& port);
    Endpoint(const std::string& str);

    bool equals(const Endpoint& other) const;
    friend bool operator==(const Endpoint& lhs, const Endpoint& rhs);

    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Endpoint& obj);
    std::string address() const;

    std::string protocol;
    std::string machine;
    std::string port;
};
}
}
