#ifndef GROUPMODEL_HPP
#define GROUPMODEL_HPP

#include "group.hpp"

class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    bool addGroup(int userid, int groupid, string role);
    // 查询用户所在群组的信息(查询用户加了多少群)
    vector<Group> query(int userid);
    // 根据指定的groupid查询群组用户id列表，除了userid自己，主要用户群聊业务给群组其他成员群发消息(群聊业务)
    vector<int> queryGroupUsers(int userid, int groupid);
};

#endif