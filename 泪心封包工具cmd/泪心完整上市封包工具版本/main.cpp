#include "tear.h"
 
//泪心电报 t.me/TearGame




// DLL 主函数
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {

    
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        
       
        // 打开调试控制台
       OpenConsole();
   
       
      
       std::cout << "[Info]注入生成后的配置文件在花瓶软件根目录！请修改根目录tear.conf文件！" << std::endl;
    
         //加载核心配置
       if(LoadConfiguration())
       {
       }
       else { 
        
           printf("配置文件错误或缺少值!");
          
           system("pause");
           exit(0); }

        // Detours 开始 Hook
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        // 获取原始函数地址
          
        OriginalSend = (SEND)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "send");
        OriginalRecv = (RECV)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "recv");
        OriginalSendTo = (SENDTO)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "sendto");
        OriginalRecvFrom = (RECVFROM)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "recvfrom");
        OriginalWSASend = (WSASEND)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "WSASend");
        OriginalWSARecv = (WSARECV)GetProcAddress(GetModuleHandleA("Ws2_32.dll"), "WSARecv");
        /* 这里也可以换种方法获取句柄 简化代码片段
        HMODULE hModule = GetModuleHandleA("Ws2_32.dll");  然后把GetModuleHandleA("Ws2_32.dll") 改该变量
        */
        if (OriginalSend && OriginalRecv) {
            std::cout << "[泪心封包] 2.0完整版 递进封包 发送接收替换全部写完了！比起始之家稳定 零炸花瓶！" << std::endl;
            // 设置 Hook
           

            if (judgesend == 1)
            {
                DetourAttach(&(PVOID&)OriginalSend, HookedSend);
              
                printf("[泪心]安装发送状态：只改发送封包！\n");
                
            }
            if (judgerecv == 1) {
                DetourAttach(&(PVOID&)OriginalRecv, HookedRecv);
          
                printf("[泪心]安装接收状态：只改服务器接收到的封包！\n");
              
            }

            //旧方法  依附什么函数就hook劫持某个封包数据拦截功能！ 
          //  DetourAttach(&(PVOID&)OriginalSendTo, HookedSendTo);
         //   DetourAttach(&(PVOID&)OriginalRecvFrom, HookedRecvFrom);
         //   DetourAttach(&(PVOID&)OriginalWSARecv, HookedWSARecv);
          //  DetourAttach(&(PVOID&)OriginalWSASend, HookedWSASend);

        }
        else {
            std::cerr << "[泪心封包] 无法定位到函数功能 请确定是windows系统!" << std::endl;
        }

        LONG error = DetourTransactionCommit();
        if (error == NO_ERROR) {
            std::cout << "[泪心封包] 正式版本没有控制台调试输出！防止卡顿！请看花瓶数据!" << std::endl;
        }
        else {
            std::cerr << "[泪心封包] 劫持安装失败: " << error << std::endl;
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        // 输出调试信息
        std::cout << "[WinsockHook]正在卸载劫持API接口中..." << std::endl;
       
        // Detours 移除 Hook
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)OriginalSend, HookedSend);
        DetourDetach(&(PVOID&)OriginalRecv, HookedRecv);
        DetourDetach(&(PVOID&)OriginalSend, HookedSendTo);
        DetourDetach(&(PVOID&)OriginalRecv, HookedRecvFrom);
        DetourDetach(&(PVOID&)OriginalWSARecv, HookedWSARecv);
        DetourDetach(&(PVOID&)OriginalWSASend, HookedWSASend);
        DetourTransactionCommit();
        
        std::cout << "[WinsockHook] 完美移除hook网络接口!" << std::endl;
    }
    return TRUE;
}

