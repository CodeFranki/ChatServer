#ifndef DROUP_HPP
#define GROUP_HPP

#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;

class Group
{

public:
    // 初始化群
    Group(int id = -1, string name = "", string desc = "")
    {
        this->id = id;
        this->name = name;
        this->desc = desc;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setDesc(string desc) { this->desc = desc; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getDesc() { return this->desc; }
    vector<GroupUser> &getUsers() { return this->users; }

private:
    int id;
    string name;
    string desc;             // 群的描述（群简介）
    vector<GroupUser> users; // 用来存储群成员
};

#endif