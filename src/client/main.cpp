#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <sstream>
#include <chrono> // 和ctime用来获取系统时间
#include <ctime>
#include <vector>
#include <semaphore.h>
#include <atomic>
using namespace std;

#include "json.hpp"
#include "public.hpp"
#include "group.hpp"
#include "user.hpp"
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;

// 记录登录状态,初始化为false
atomic_bool g_isLoginSuccess{false};

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间(聊天信息需要添加时间)
string getCurrentTime();
// 显示当前登录成功用户的基本信息
void showCurrentUserData();
// 主聊天页面程序
void mainMenu(int clientfd);

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接受线程
int main(int argc, char **argv)
{
    // 输入的参数如果小于3个
    if (argc < 3)
    {
        cerr << "command invalid! Example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析用过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket create error!" << endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 填写client 和 server进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect client error!" << endl;
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    sem_init(&rwsem, 0, 0);

    // 连接服务器成功，启动接收子线程
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    // main线程用于接收用户输入，负责发送数据
    for (;;)
    {
        // 显示首页菜单  登录，注册，退出
        cout << "===========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "===========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice; // 自己看着是输入了value，但是实际上是value\n,后面会将value读出，缓冲区仅剩\n，如果后面需要读char，可能会将这个\n读出，会造成错误
        cin.get();     // 读掉缓冲区残留的回车

        switch (choice)
        {
        case 1: // 登录业务
        {
            int id = 0;
            char password[50] = {0};
            cout << "id:";
            cin >> id;
            cin.get();
            cout << "password:";
            cin.getline(password, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = password;
            string request = js.dump();

            g_isLoginSuccess = false;

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (-1 == len)
            {
                cerr << "send login msg error!" << endl;
            }

            sem_wait(&rwsem); // 等待信号量，由子线程处理完登陆的响应消息后，通知这里

            if (g_isLoginSuccess)
            {
                // 进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        }
        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char password[50] = {0};
            cout << "username:";
            cin.getline(name, 50); // cin>> 默认是遇见空格或者换行就结束，为了避免zhang san情况，getline只遇见换行
            cout << "password:";
            cin.getline(password, 50);

            // 组装注册的json
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = password;
            string request = js.dump(); // json需要转成string才能在网络里面传输

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (-1 == len)
            {
                cerr << "send reg msg error!" << endl;
            }

            sem_wait(&rwsem); // 等待信号量，子线程处理完会通知
        }
        break;
        case 3: // 退出业务
        {
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        }
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
    return 0;
}

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群聊，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群聊，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 主聊天界面
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};

    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024); // 获取命令
        string commandbuf(buffer); // 是格式命令，例如help或者chat:friendid:message
        string command;            // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx) // 命令里面没有：，即help和loginout，那就直接将命令给command
        {
            command = commandbuf;
        }
        else
        {                                        // 如果找见了，也就是格式是chat:friendid:message，那命令就是：前面的
            command = commandbuf.substr(0, idx); // 提取命令
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用响应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法，将friendid:message按照string类型传入，在函数里面详细处理，回应
    }
}

// "help" command handler
void help(int, string)
{
    cout << "show command list >>>" << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send msg error -> " << buffer << endl;
    }
}

// "addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error!" << endl;
    }
}

// "creategroup" command handler
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "create group command invalid!" << endl;
        return;
    }
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send create group msg error!" << endl;
    }
}

// "addgroup" command handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error!" << endl;
    }
}

// "groupchat" command handler
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string msg = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = msg;
    js["time"] = getCurrentTime();
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error!" << endl;
    }
}

// "loginout" command handler
void loginout(int clientfd, string)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buff = js.dump();

    int len = send(clientfd, buff.c_str(), strlen(buff.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error! -> " << buff << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

void doRegResponse(json responsejs)
{
    if (0 != responsejs["errno"].get<int>()) // 注册失败
    {
        cerr << "name is already exist,register error!" << endl;
    }
    else
    { // 注册成功
        cout << "name register success,userid is " << responsejs["id"] << ",do not forget it!" << endl;
    }
}

// 处理登陆的响应
void doLoginResponse(json responsejs)
{
    if (0 != responsejs["errno"])
    { // 登录失败
        cerr << responsejs["msg"] << endl;
        g_isLoginSuccess = false;
    }
    else
    { // 登录成功
        // 记录当前用户的信息
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends"))
        {
            // 初始化,如果没有初始化，执行loginout之后，再次向vector中插入数据
            g_currentUserFriendList.clear();

            vector<string> vec = responsejs["friends"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);

                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if (responsejs.contains("groups"))
        {
            // 初始化
            g_currentUserGroupList.clear();

            vector<string> vec1 = responsejs["groups"];
            for (string &groupStr : vec1)
            {
                json grpjs = json::parse(groupStr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                vector<string> vec2 = grpjs["users"];
                for (string userStr : vec2)
                {
                    GroupUser user;
                    json js = json::parse(userStr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线消息，个人聊天或者群组的离线消息
        if (responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for (string &str : vec)
            {
                json js = json::parse(str);
                int msgtype = js["msgid"].get<int>();
                if (ONE_CHAT_MSG == msgtype)
                {
                    cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
                }

                if (GROUP_CHAT_MSG == msgtype)
                {
                    cout << "群消息[" << js["groupid"] << "]: " << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSuccess = true;
    }
}

// 子线程-接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }

        // 接收ChatServer发送的数据，反序列化生成json对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]: " << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>() << " said:" << js["msg"].get<string>() << endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype)
        {
            doLoginResponse(js); // 处理登录响应的业务逻辑
            sem_post(&rwsem);    // 通知主线程，登录结果处理完成
            continue;
        }

        if (REG_MSG_ACK == msgtype)
        {
            doRegResponse(js); // 处理注册响应的业务逻辑
            sem_post(&rwsem);  // 通知主线程，注册结果处理完成
            continue;
        }
    }
}
// // 获取系统时间(聊天信息需要添加时间)
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "================================login================================" << endl;
    cout << "current login user:" << g_currentUser.getId() << " " << "name:" << g_currentUser.getName() << endl;
    cout << "------------------------------Friend List-----------------------------" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << "id:" << user.getId() << " name:" << user.getName() << " state:" << user.getState() << endl;
        }
    }
    cout << "------------------------------Group List------------------------------" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group group : g_currentUserGroupList)
        {
            cout << group.getId() << " " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << endl;
            }
        }
    }
    cout << "----------------------------------------------------------------------" << endl;
}
