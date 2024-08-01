#include "usermodel.hpp"
#include "db.hpp"

bool UserModel::insert(User &user)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id,因为这个id是数据库自行创建的，再讲这个id拿出来给user.id设置
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id=%d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res); // 根据id查出来的结果是一行数据，可以像数组一样访问每一列
            if (row != nullptr)
            {
                User user; // 创建了一个User对象，将查询出来的结果存到对象中
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res); // 将mysql查询出来的结果释放
                return user;
            }
        }
    }
    return User(); // 如果查询失败，就返回User的默认构造函数的对象（空对象）
}

bool UserModel::updateState(User user)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update User set state='%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024] = "update User set state='offline' where state='online'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}