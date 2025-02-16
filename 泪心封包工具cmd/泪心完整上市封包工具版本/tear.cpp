#include "tear.h"

//泪心电报 t.me/TearGame
// 初始化原始函数指针
SEND OriginalSend = nullptr;
RECV OriginalRecv = nullptr;
SENDTO OriginalSendTo = nullptr;    //发送到数据 发给服务器
RECVFROM OriginalRecvFrom = nullptr;   //接收到数据  来自服务器
WSASEND  OriginalWSASend = nullptr;
WSARECV  OriginalWSARecv = nullptr;

  std::vector<unsigned  char> findhexpacker;    //寻找封包字节集
  std::vector<unsigned  char> replacehexpacker;   //替换字节集
  int changepos;  // 替换位置数组
  int incresmentvalue; // 递进值数组
    int judgesend;     //是否启动发送Hook
    int  judgerecv; //是否启动接收Hook
  // 配置文件路径定义
  const char* CONFIG_FILE = "tear.conf";

// 打开一个控制台窗口
#include <windows.h>
#include <iostream>

// 打开控制台窗口并设置属性
  void OpenConsole() {
      // 分配控制台
      if (!AllocConsole()) {
          MessageBoxA(NULL, "无法分配内存打开控制台!", "Error", MB_ICONERROR);
          return;
      }

      // 设置控制台标题
      SetConsoleTitleA("泪心认证独研封包工具专享版权QQ2254013571 电报@TearGame");

      // 重定向标准输出到控制台
      FILE* fConsole;
      freopen_s(&fConsole, "CONOUT$", "w", stdout); // 重定向标准输出到控制台
      freopen_s(&fConsole, "CONIN$", "r", stdin);  // 重定向标准输入到控制台
      freopen_s(&fConsole, "CONOUT$", "w", stderr); // 重定向标准错误到控制台

      // 清除流状态
      std::cout.clear();
      std::cin.clear();
      std::clog.clear();
      std::cerr.clear();

      // 设置控制台文本为绿色
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
          SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
      }

      // 测试输出
      std::cout << "[Info] 泪心封包工具显示中控终端面板! 泪心开发人员内部特供封包工具！" << std::endl;
  }

 

  // 工具函数：去除字符串首尾空白
  std::string trim(const std::string& str) {
      size_t first = str.find_first_not_of(" \t\r\n");
      size_t last = str.find_last_not_of(" \t\r\n");
      return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
  }

  // 工具函数：将十六进制字符串转换为字节
  unsigned char hexToByte(const std::string& str) {
      return static_cast<unsigned char>(std::stoi(str, nullptr, 16));
  }

 
  // 配置文件加载函数   方便用户修改封包内容!
  bool LoadConfiguration() {
      std::ifstream configFile(CONFIG_FILE);
      if (!configFile.is_open()) {
          std::cerr << "无法打开配置文件: " << CONFIG_FILE << std::endl;
          return false;
      }

      std::unordered_map<std::string, std::string> config;
      std::string line;

      // 逐行解析配置文件
      while (std::getline(configFile, line)) {
          if (line.empty() || line[0] == '#') continue; // 跳过注释和空行

          std::istringstream lineStream(line);
          std::string key, value;
          if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
              key = trim(key);
              value = trim(value);
              config[key] = value;
          }
      }

      // 检查配置文件完整性
      if (config.find("leixinTargerHeaderHex") == config.end() ||
          config.find("leixinReplacement") == config.end() ||
          config.find("leixinposition") == config.end() ||
          config.find("leixindijin") == config.end()) {
          std::cerr << "配置文件缺少必要的键值。" << std::endl;
         
          return false;
      }

      try {
          // 解析 leixinTargerHeaderHex
          findhexpacker.clear();
          std::istringstream targetStream(config["leixinTargerHeaderHex"]);
          std::string hexValue;
          while (targetStream >> hexValue) {
              findhexpacker.push_back(hexToByte(hexValue));
          }

          // 解析 leixinReplacement
          replacehexpacker.clear();
          std::istringstream replacementStream(config["leixinReplacement"]);
          while (replacementStream >> hexValue) {
              replacehexpacker.push_back(hexToByte(hexValue));
          }

            //解析替换位置
          std::string posValue = trim(config["leixinposition"]);
          changepos = static_cast<int>(strtol(posValue.c_str(), nullptr, 16));
          // 使用 C 的 strtol 按十进制解析  很恶心人啊 相反我去！
          //   16 代表转换的最终值是十进制   然后 10 代表最终转换的值是十六进制
          
          //解析递进值  位置
          std::string   dijinStringvalue = trim(config["leixindijin"]);
          incresmentvalue= static_cast<int>(strtol(dijinStringvalue.c_str(), nullptr, 16));

          //是否开关接收
          std::string   receivestring = trim(config["recv"]);
          judgerecv = static_cast<int>(strtol(receivestring.c_str(), nullptr, 16));
          //是否开关发送
          std::string   sendstring = trim(config["send"]);
          judgesend = static_cast<int>(strtol(sendstring.c_str(), nullptr, 16));


        
      }
      catch (const std::exception& e) {
          std::cerr << "解析配置文件时发生错误: " << e.what() << std::endl;
          return false;
      }

      // 打印加载结果
      std::cout << "配置加载成功: " << std::endl;

      std::cout << "leixinTargerHeaderHex: ";
      for (auto val : findhexpacker) {
          std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)val << " ";
      }
      std::cout << std::endl;

      std::cout << "leixinReplacement: ";
      for (auto val : replacehexpacker) {
          std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)val << " ";
      }
      std::cout << std::endl;

      //正常开始封包不建议启动调试功能！
      std::cout << "泪心替换封包位置 : " << changepos << std::endl;
      std::cout << "泪心递进值: " << incresmentvalue << std::endl;
      std::cout << "发送和接收开关状态(0关1开) 发送= " << judgesend << "和  接收 = " << judgerecv << std::endl << std::endl;
      return true;
  }
 
  //~~~~~~~~~~~~~~~~~~~~~~~开始泪心捕获发送出去的软件封包操作~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  /*
  int WSAAPI HookedSend(SOCKET s, const char* buf, int len, int flags)
  {
      return  OriginalSend(s, buf, len, flags);
  }
    */

  //只改发送！
  int WSAAPI HookedSend(SOCKET s, const char* buf, int len, int flags) {
      // 创建一个临时缓冲区，用于修改数据
      std::vector<char> tempBuffer(buf, buf + len);

      // 检查是否找到寻找的封包字节集
      auto it = std::search(tempBuffer.begin(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      while (it != tempBuffer.end()) {
          // 在找到的位置替换字节集，确保我们不会访问超过了 vector 结束位置的迭代器
          if (std::distance(it, tempBuffer.end()) >= changepos + replacehexpacker.size()) {
              std::copy(replacehexpacker.begin(), replacehexpacker.end(), it + changepos);

              // 输出调试信息
             // printf("成功在 %d 位置替换封包,封包长度是 %d\n", std::distance(tempBuffer.begin(), it + changepos), replacehexpacker.size());
          }

          // 递进值算法，确保我们不会访问超过了 vector 结束位置的迭代器
          if (std::distance(it, tempBuffer.end()) >= changepos + incresmentvalue) {
              *(it + changepos) += incresmentvalue;
          }

          // 继续寻找下一个匹配的字节集
          it = std::search(it + findhexpacker.size(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      }

      // 调用原始 send 函数
      return OriginalSend(s, tempBuffer.data(), tempBuffer.size(), flags);
  }


/*  稳定封包真正开源者  泪心   电报 t.me/TearGame   QQ频道 点击链接加入腾讯频道【泪心and 独久 love频道】：https://pd.qq.com/s/estiees5z
  //认证泪心方案   稳定连打王者封包!!!
  int WSAAPI HookedSend(SOCKET s, const char* buf, int len, int flags) {
      // 创建缓冲区副本
      char* modifiedBuf = new char[len];
      memcpy(modifiedBuf, buf, len);

      // 要查找的目标字节序列
      const char target[] = { 0x00,0x00,0x0A,0x11 };
      const int targetLen = sizeof(target);

      // 替换内容
      const char replacement[] = { 0xFF,0xFF  };
      const int replacementLen = sizeof(replacement);

      // 遍历缓冲区递进查找目标字节序列
      int replaceCount = 0; // 替换计数器
      for (int i = 0; i <= len - targetLen; ++i) {
          if (memcmp(modifiedBuf + i, target, targetLen) == 0) {
              // 计算替换偏移量，递进位置增加，例如每次替换后递增 5 字节
              int replaceOffset = 56+ (replaceCount *999); // 递进偏移量

              // 确保替换位置在缓冲区范围内
              if (i + replaceOffset + replacementLen <= len) {
                  memcpy(modifiedBuf + i + replaceOffset, replacement, replacementLen);
                  ++replaceCount; // 记录替换次数
              }
              else {
                  // 超出缓冲区范围，停止进一步替换
                //  std::cerr << "【NEW】2.0版本递进替换超出缓冲区范围，停止替换" << std::endl;
                  break;
              }
          }
      }

      // 调用原始 send 函数发送修改后的数据
      int result = OriginalSend(s, modifiedBuf, len, flags);

      // 释放分配的内存
      delete[] modifiedBuf;

      // 打印替换信息
   //   std::cout << "总共执行了 " << replaceCount << " 次替换操作。" << std::endl;

      return result;
  }

  */

  /*
  int WSAAPI HookedRecv(SOCKET s, char* buf, int len, int flags) {
      return  OriginalRecv(s, buf, len, flags);
  }
  */



  //~~~~~~~~~~~~~~~~~~~~~~~开始泪心捕获接收到的软件封包操作~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //只改接收
  int WSAAPI HookedRecv(SOCKET s, char* buf, int len, int flags) {
      // 调用原始 recv 函数接收数据
      int ret = OriginalRecv(s, buf, len, flags);
      if (ret <= 0) {
          return ret; // 如果接收失败或连接关闭，直接返回
      }

      // 创建一个临时缓冲区，用于修改接收到的数据
      std::vector<char> tempBuffer(buf, buf + ret);

      // 检查是否找到寻找的字节模式
      auto it = std::search(tempBuffer.begin(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      while (it != tempBuffer.end()) {
          // 在找到的位置替换字节集，确保不越界
          if (std::distance(it, tempBuffer.end()) >= changepos + replacehexpacker.size()) {
              std::copy(replacehexpacker.begin(), replacehexpacker.end(), it + changepos);

              // 输出调试信息
           //   printf("接收替换中===成功在位置 %d 替换封包，封包长度是 %d\n", std::distance(tempBuffer.begin(), it + changepos), replacehexpacker.size());
          }

          // 如果递增值条件满足，修改指定字节  递进都已经为您写好了
          if (std::distance(it, tempBuffer.end()) >= changepos + incresmentvalue) {
              *(it + changepos) += incresmentvalue;

              // 输出调试信息
            //  printf("位置 %d 的字节值增加了 %d\n", std::distance(tempBuffer.begin(), it + changepos), incresmentvalue);
          }

          // 继续寻找下一个匹配的字节集
          it = std::search(it + findhexpacker.size(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      }

      // 将修改后的数据拷贝回原始缓冲区
      std::copy(tempBuffer.begin(), tempBuffer.end(), buf);

      return ret; // 返回接收到的数据长度
  }

 

    //端口作者看清楚 你起始之家WPE处理什么发送到  接受自  等于自慰  nj走的是tcp流量  恐怕起始之家作者都不懂
    //起始之家作者怕是自己不懂代码 不懂协议https  连最基本的注释都不告诉使用者  还得泪心来讲课
    //接收来自   发送到 是处理UDP协议的 所以请你无视吧  传过来的数据都是TCP协议  处理自慰的UDP啊 ！！
  int WINAPI HookedSendTo(SOCKET s, const char* buf, int len, int flags, const sockaddr* to, int tolen) {
     // std::cout << "HookedSendTo: " << len << " bytes sent to." << std::endl;
      return OriginalSendTo(s, buf, len, flags, to, tolen);
  }


    int WSAAPI HookedRecvFrom( SOCKET s,char* buf,int len,int flags,struct sockaddr* from,int* fromlen) {
      // 调用原始 recvfrom 函数接收数据
      int ret = OriginalRecvFrom(s, buf, len, flags, from, fromlen);
      if (ret <= 0) {
          return ret; // 如果接收失败或连接关闭，直接返回
      }

      // 创建一个临时缓冲区，用于修改接收到的数据
      std::vector<char> tempBuffer(buf, buf + ret);

      // 检查是否找到寻找的字节模式
      auto it = std::search(tempBuffer.begin(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      while (it != tempBuffer.end()) {
          // 在找到的位置替换字节集，确保不越界
          if (std::distance(it, tempBuffer.end()) >= changepos + replacehexpacker.size()) {
              std::copy(replacehexpacker.begin(), replacehexpacker.end(), it + changepos);

              // 输出调试信息
              printf("来自服务器接收  成功在位置 %d 替换封包，封包长度是 %d\n", std::distance(tempBuffer.begin(), it + changepos), replacehexpacker.size());
          }

          // 如果递增值条件满足，修改指定字节
          if (std::distance(it, tempBuffer.end()) >= changepos + incresmentvalue) {
              *(it + changepos) += incresmentvalue;

              // 输出调试信息
           //   printf("来自服务器接收   位置 %d 的字节值增加了 %d\n", std::distance(tempBuffer.begin(), it + changepos), incresmentvalue);
          }

          // 继续寻找下一个匹配的字节集
          it = std::search(it + findhexpacker.size(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
      }

      // 将修改后的数据拷贝回原始缓冲区
      std::copy(tempBuffer.begin(), tempBuffer.end(), buf);

      return ret; // 返回接收到的数据长度
  }



 
    int WINAPI HookedWSARecv(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesRecvd,LPDWORD lpFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) 
    {
        // 调用原始 WSARecv 函数
        int ret = OriginalWSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine);

        if (ret != 0 || lpNumberOfBytesRecvd == nullptr || *lpNumberOfBytesRecvd <= 0) {
            return ret; // 如果接收失败或无数据，直接返回
        }

        // 遍历缓冲区，处理接收的数据
        for (DWORD i = 0; i < dwBufferCount; ++i) {
            LPWSABUF currentBuffer = &lpBuffers[i];
            std::vector<char> tempBuffer(currentBuffer->buf, currentBuffer->buf + currentBuffer->len);

            // 查找字节模式
            auto it = std::search(tempBuffer.begin(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
            while (it != tempBuffer.end()) {
                // 替换字节集
                if (std::distance(it, tempBuffer.end()) >= changepos + replacehexpacker.size()) {
                    std::copy(replacehexpacker.begin(), replacehexpacker.end(), it + changepos);

                    // 输出调试信息
                    printf("2.0封包方式接收成功在位置 %d 替换封包，封包长度是 %d\n", std::distance(tempBuffer.begin(), it + changepos), replacehexpacker.size());
                }

                // 如果递增值条件满足，修改指定字节
                if (std::distance(it, tempBuffer.end()) >= changepos + incresmentvalue) {
                    *(it + changepos) += incresmentvalue;

                    // 输出调试信息
                    printf("位置 %d 的字节值增加了 %d\n", std::distance(tempBuffer.begin(), it + changepos), incresmentvalue);
                }

                // 查找下一个匹配的字节集
                it = std::search(it + findhexpacker.size(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
            }

            // 将修改后的数据写回原始缓冲区
            std::copy(tempBuffer.begin(), tempBuffer.end(), currentBuffer->buf);
        }

        return ret; // 返回接收到的数据长度
    }


    //WSAsend函数  
    int WINAPI HookedWSASend(SOCKET s,LPWSABUF lpBuffers,DWORD dwBufferCount,LPDWORD lpNumberOfBytesSent,DWORD dwFlags,LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) 
    {
        // 遍历缓冲区，处理待发送的数据
        for (DWORD i = 0; i < dwBufferCount; ++i) {
            LPWSABUF currentBuffer = &lpBuffers[i];
            std::vector<char> tempBuffer(currentBuffer->buf, currentBuffer->buf + currentBuffer->len);

            // 查找字节模式
            auto it = std::search(tempBuffer.begin(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
            while (it != tempBuffer.end()) {
                // 替换字节集
                if (std::distance(it, tempBuffer.end()) >= changepos + replacehexpacker.size()) {
                    std::copy(replacehexpacker.begin(), replacehexpacker.end(), it + changepos);

                    // 输出调试信息
                    printf("成功在位置 %d 替换封包，封包长度是 %d\n", std::distance(tempBuffer.begin(), it + changepos), replacehexpacker.size());
                }

                // 如果递增值条件满足，修改指定字节
                if (std::distance(it, tempBuffer.end()) >= changepos + incresmentvalue) {
                    *(it + changepos) += incresmentvalue;

                    // 输出调试信息
                    printf("位置 %d 的字节值增加了 %d\n", std::distance(tempBuffer.begin(), it + changepos), incresmentvalue);
                }

                // 查找下一个匹配的字节集
                it = std::search(it + findhexpacker.size(), tempBuffer.end(), findhexpacker.begin(), findhexpacker.end());
            }

            // 将修改后的数据写回原始缓冲区
            std::copy(tempBuffer.begin(), tempBuffer.end(), currentBuffer->buf);
        }

        // 调用原始 WSASend 函数
        return OriginalWSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);   
    }


