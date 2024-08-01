#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include "user.hpp"

class UserModel
{
public:
    // User表的增加方法
    bool insert(User &user);

    // 根据用户的id对用户信息进行查询
    User query(int id);

    // 更新用户的状态信息
    bool updateState(User user);

    // 重置用户的状态
    void resetState();
};

#endif