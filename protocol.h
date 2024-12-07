#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include<QString.h>
#include <unistd.h>
#define msg_ok "succes"
#define msg_failed "failed"
enum msgtype
{
    MSG_TYPE_MIN=0,
    MSG_TYPE_REGIST_REQUEST,//注册请求
    MSG_TYPE_REGIST_RESPOND,//注册响应
    MSG_TYPE_LOGIN_REQUEST,//登录请求
    MSG_TYPE_LOGIN_RESPOND,//登录响应
    MSG_TYPE_DEL_REQUEST,//注销请求
    MSG_TYPE_DEL_RESPOND,//注销响应
    MSG_TYPE_EXIT_REQUEST,
    MSG_TYPE_ONLINE_REQUEST,//在线好友请求
    MSG_TYPE_ONLINE_RESPOND,//在线好友响应
    MSG_TYPE_SEARCHUSR_REQUEST,//搜索用户请求
    MSG_TYPE_SEARCHUSR_RESPOND,//搜索用户响应
    MSG_TYPE_FRIEND_REQUEST,//好友请求
    MSG_TYPE_FRIEND_RESPOND,//好友响应
    MSG_TYPE_AGREE_FRIEND_RESPOND,//接收好友请求响应
    MSG_TYPE_DISAGREE_FRIEND_RESPOND,//拒绝好友请求响应
    MSG_TYPE_REFREASH_REQUEST,//刷新好友请求
    MSG_TYPE_REFREASH_RESPOND,//刷新好友响应
    MSG_TYPE_DELFRIEND_REQUEST,//删除好友请求
    MSG_TYPE_DELFRIEND_REPOND,//删除好友响应
    MSG_TYPE_PRIVATECHAT_REQUEST,//私聊请求
    MSG_TYPE_AGREE_PRIVATECHAT_RESPOND,//同意私聊响应
    MSG_TYPE_DISAGREE_PRIVATECHAT_RESPOND,//拒绝私聊响应
    MSG_TYPE_CHAT,//聊天
    MSG_TYPE_CREATE_DIR_REQUEST,//创建文件夹请求
    MSG_TYPE_CREATE_DIR_RESPOND,//创建文件夹响应
    MSG_TYPE_FLUSH_FILE_REQUEST,//刷新文件夹请求
    MSG_TYPE_FLUSH_FILE_RESPOND,//刷新文件夹响应
    MSG_TYPE_DEL_DIR_REQUEST,//删除文件夹请求
    MSG_TYPE_DEL_DIR_RESPOND,//删除文件夹响应
    MSG_TYPE_RENAME_DIR_REQUEST,//重命名文件夹请求
    MSG_TYPE_RENAME_DIR_RESPOND,//重命名文件夹响应
    MSG_TYPE_ENTER_DIR_REQUEST,//进入文件夹请求
    MSG_TYPE_ENTER_DIR_RESPOND,//进入文件夹响应
    MSG_TYPE_RETURN_DIR_REQUEST,//返回上一级请求
    MSG_TYPE_RETURN_DIR_RESPOND,//返回上一级响应
    MSG_TYPE_UPLOAD_FILE_REQUEST,//上传文件请求
    MSG_TYPE_UPLOAD_FILE_RESPOND,//上传文件响应
    MSG_TYPE_UPLOAD_FINISH_RESPOND,//上传文件完成
    MSG_TYPE_DEL_FILE_REQUEST,//删除文件请求
    MSG_TYPE_DEL_FILE_RESPOND,//删除文件响应
    MSG_TYPE_DOWNLOAD_FILE_REQUEST,//下载文件请求
    MSG_TYPE_DOWNLOAD_FILE_RESPOND,//下载文件响应
    MSG_TYPE_MAX=0xFFFFFFFF,
};
struct MFileInfo
{
    char caName[32];//文件名字
    int FileType;//文件类型
};
struct PDU
{
    unsigned int PDUlen;//整个消息的大小
    unsigned int msgType;//消息类型
    char cData[64];//文件名
    unsigned int msglen;//文件的大小
    int msg[];//弹性控制大小
};
PDU* mkPDU(unsigned int msglen);
#endif // PROTOCOL_H
