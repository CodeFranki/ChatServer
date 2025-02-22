#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP

#include <vector>
using namespace std;

#include "user.hpp"

// 维护好友信息的操作接口方法
class FriendModel
{
public:
    // 添加好友关系
    void insert(int userid, int friendid);

    // 返回好友列表信息
    vector<User> query(int userid);
};

#endif