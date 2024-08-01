#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP

#include "user.hpp"
#include <string>

class GroupUser : public User // 群成员也是普通的用户，只是他多了一个身份
{
public:
    // 设置群成员的属性
    void setRole(string role) { this->role = role; }

    // 获取群成员的属性
    string getRole() { return this->role; }

private:
    // 是成员还是管理员
    string role;
};

#endif