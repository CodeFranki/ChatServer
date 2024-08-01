#ifndef USER_HPP
#define USER_HPP

#include <string>
using namespace std;

// ORM（Object-Relational Mapping，对象关系映射）是一种编程技术，
// 用于将面向对象编程语言中的类与数据库中的表进行映射，
// 允许开发者使用对象的方式来操作数据库，而无需编写大量的SQL语句。

// 匹配User表的ORM类
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};

#endif