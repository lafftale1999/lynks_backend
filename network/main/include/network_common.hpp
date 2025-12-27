#ifndef COMMON_HEADERS_HPP_
#define COMMON_HEADERS_HPP_

#include <iostream>
#include <memory>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <optional>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <map>

#define BOOST_CHARCONV_HEADER_ONLY

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

namespace http = boost::beast::http;
using http_request = http::request<http::string_body>;
using http_response = http::response<http::string_body>;

namespace asio = boost::asio;

#endif