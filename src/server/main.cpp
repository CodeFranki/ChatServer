#include <iostream>
#include <string>
#include <signal.h>
#include "chatserver.hpp"
#include "chatservice.hpp"
using namespace std;

// 处理服务器Ctrl+C结束之后，重置user的状态信息
void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    // 输入的参数如果小于3个
    if (argc < 3)
    {
        cerr << "command invalid! Example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}
