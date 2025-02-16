#pragma once
#include<iostream>
#include<stdio.h>
#include<WinSock2.h>
#include<detours.h>
#include<Windows.h>
#include <mutex>
#include<fstream>   //文本写入内容  支持头
#include <iomanip>   //十六进制支持头 格式化输出
#include<cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <optional>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "X64tear.lib")
#pragma comment(lib, "X86tear.lib")
 

//泪心定义核心变量 很重要的！！！   寻找特征  替换字节集  替换位置  递进位置
extern std::vector<unsigned  char> findhexpacker;
extern std::vector<unsigned  char> replacehexpacker;
extern int changepos;  // 替换位置数组
extern int incresmentvalue; // 递进值数组
extern int judgesend;
extern int  judgerecv;
// 配置文件路径声明
extern const char* CONFIG_FILE;


// 函数指针类型声明  需要hook和劫持的函数 
typedef int (WINAPI* SEND)(SOCKET, const char*, int, int);
typedef int (WINAPI* RECV)(SOCKET, char*, int, int);
typedef int (WINAPI* SENDTO)(SOCKET, const char*, int, int, const sockaddr*, int);
typedef int (WINAPI* RECVFROM)(SOCKET, char*, int, int, sockaddr*, int*);
// WSASend 函数类型
typedef int (WINAPI* WSASEND)(
    SOCKET,                      // 描述符
    LPWSABUF,                    // 数据缓冲区指针
    DWORD,                       // 缓冲区数量
    LPDWORD,                     // 成功发送的字节数
    DWORD,                       // 发送标志
    LPWSAOVERLAPPED,             // 异步操作对象
    LPWSAOVERLAPPED_COMPLETION_ROUTINE // 完成例程
    );
// WSARecv 函数类型
typedef int (WINAPI* WSARECV)(
    SOCKET,                      // 描述符
    LPWSABUF,                    // 数据缓冲区指针
    DWORD,                       // 缓冲区数量
    LPDWORD,                     // 成功接收的字节数
    LPDWORD,                     // 接收标志
    LPWSAOVERLAPPED,             // 异步操作对象
    LPWSAOVERLAPPED_COMPLETION_ROUTINE // 完成例程
    );
// 原始函数指针
extern SEND OriginalSend;
extern RECV OriginalRecv;
extern SENDTO OriginalSendTo;
extern RECVFROM OriginalRecvFrom;
// 声明原始函数指针
extern WSASEND OriginalWSASend;
extern WSARECV OriginalWSARecv;

// Hook 函数声明
int WSAAPI HookedSend(SOCKET s, const char* buf, int len, int flags);
int WSAAPI HookedRecv(SOCKET s, char* buf, int len, int flags);
//追加 发送到 接收到（接受来自服务器）
int WINAPI HookedSendTo(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen);
int WINAPI HookedRecvFrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen);
//WSAsend函数
int WINAPI HookedWSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
int WINAPI HookedWSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);




//声明调试窗口函数
void OpenConsole();
//声明配置函数
bool LoadConfiguration();


//核心反破解函数 须知
int Hiden();