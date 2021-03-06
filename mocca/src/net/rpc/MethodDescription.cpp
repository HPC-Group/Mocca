/*The MIT License(MIT)

Copyright(c) 2016 David McCann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the
"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to
the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "mocca/net/rpc/MethodDescription.h"

#include "mocca/net/rpc/ValueType.h"

#include "mocca/base/Error.h"

using namespace mocca::net;

MethodDescription::ParameterDescription::ParameterDescription(const std::string name, JsonCpp::ValueType type,
                                                              const JsonCpp::Value& defaultValue)
    : name_(name)
    , type_(type)
    , defaultValue_(defaultValue) {
    if (!defaultValue_.empty()) {
        if (type_ != defaultValue_.type()) {
            throw Error("Type of default value does not match parameter type", __FILE__, __LINE__);
        }
    }
}

std::string MethodDescription::ParameterDescription::name() const {
    return name_;
}

JsonCpp::ValueType MethodDescription::ParameterDescription::type() const {
    return type_;
}

JsonCpp::Value MethodDescription::ParameterDescription::defaultValue() const {
    return defaultValue_;
}

std::string MethodDescription::ParameterDescription::toString() const {
    std::stringstream stream;
    stream << "parameter: " << name_ << " (" << mocca::net::valueTypeMapper().getByFirst(type_);
    stream << "; default: ";
    if (!defaultValue_.empty()) {
        stream << defaultValue_;
    } else {
        stream << "<None>";
    }
    stream << ")";
    return stream.str();
}

bool MethodDescription::ParameterDescription::operator<(const ParameterDescription& other) const {
    return name_ < other.name_;
}


MethodDescription::MethodDescription(const std::string& name, const std::vector<ParameterDescription>& parameters)
    : name_(name)
    , parameters_(parameters) {}

std::string MethodDescription::name() const {
    return name_;
}

std::vector<MethodDescription::ParameterDescription> MethodDescription::parameters() const {
    return parameters_;
}

std::string MethodDescription::toString() const {
    std::stringstream stream;
    stream << "method: " << name_ << '\n';
    for (const auto& param : parameters_) {
        stream << '\t' << param << '\n';
    }
    return stream.str();
}

JsonCpp::Value MethodDescription::toJson(const MethodDescription& description) {
    JsonCpp::Value result;
    result["name"] = description.name();
    auto parameters = description.parameters();
    int count = 0;
    for (const auto& param : parameters) {
        result["parameters"][count]["name"] = param.name();
        result["parameters"][count]["type"] = mocca::net::valueTypeMapper().getByFirst(param.type());
        if (!param.defaultValue().empty()) {
            result["parameters"][count]["default"] = param.defaultValue();
        }
        ++count;
    }
    return result;
}

MethodDescription MethodDescription::fromJson(const JsonCpp::Value& json) {
    auto methodName = json["name"].asString();
    std::vector<MethodDescription::ParameterDescription> parameters;
    auto paramsRoot = json["parameters"];
    for (auto it = paramsRoot.begin(); it != paramsRoot.end(); ++it) {
        auto paramName = (*it)["name"].asString();
        auto type = mocca::net::valueTypeMapper().getBySecond((*it)["type"].asString());
        JsonCpp::Value defaultValue;
        if (it->isMember("default")) {
            defaultValue = (*it)["default"];
        }
        parameters.push_back({paramName, type, defaultValue});
    }
    return MethodDescription(methodName, parameters);
}

std::ostream& operator<<(std::ostream& os, const MethodDescription::ParameterDescription& obj) {
    return os << obj.toString();
}

std::ostream& operator<<(std::ostream& os, const MethodDescription& obj) {
    return os << obj.toString();
}