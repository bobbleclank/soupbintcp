#ifndef INCLUDE_SOUP_SERVER_SERVER_H
#define INCLUDE_SOUP_SERVER_SERVER_H

namespace soup::server {

class Server {
public:
  void start();
  void stop();
};

} // namespace soup::server

#endif
