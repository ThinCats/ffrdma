#include <arpa/inet.h>
#include <stdexcept>
#include <string>

struct Socket_;
typedef Socket_ Socket;

namespace ffrdma {
namespace utils {
enum class ConnStatus { OK };

std::string ipPortString(const std::string &ip, int port);

// helper function, bind and listen
Socket *createServerSocket(const std::string &ip, int port, int backlog);

std::pair<Socket *, ConnStatus> createConnectToServer(const std::string ip,
                                                      int port, int nodeId);

} // namespace utils
} // namespace ffrdma