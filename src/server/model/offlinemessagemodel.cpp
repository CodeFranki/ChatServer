#include "offlinemessagemodel.hpp"
#include "db.hpp"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert OfflineMessage values(%d,'%s')", userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return;
        }
    }
}
// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    vector<string> user;
    if (mysql.connect())
    {
        MYSQL_RES *msg = mysql.query(sql);
        if (msg != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(msg)) != nullptr)
            {                           // 离线消息不仅仅只有一条
                user.push_back(row[0]); // 这里只查询了message这个字段，所以直接是row[0]
            }
            mysql_free_result(msg);

            return user;
        }
    }
    return user;
}