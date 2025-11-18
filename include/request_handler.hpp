#pragma once

#include <string>

namespace http {

struct Request;
class Response;

class RequestHandler {
  public:
    explicit RequestHandler(const std::string& root);

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    Response handle(const Request& req);

  private:
    std::string root;
};

} // namespace http