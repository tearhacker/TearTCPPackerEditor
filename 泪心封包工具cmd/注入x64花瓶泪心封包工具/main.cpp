#include"tool.h"
#include "tearwin.h"
//请勿用于违法犯罪   知法犯法     游戏灰产属于非触碰底线  如若发现用开源项目进行贩卖 圈钱 等行为  
 //此源码已经开源 请勿二次买卖  如果你购买了 那你就被骗了  请立即投诉对方并追溯对方身份信息曝光其行为
//请注意 仅供研究目的 切勿用于违法犯罪活动！
std::atomic<bool> shouldStop(false); // 控制线程的原子变量

// 设置控制台文本颜色
void SetConsoleTextColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
// 设置窗口标题
void SetCmdTitle(const std::string& title) {
    SetConsoleTitleA(title.c_str());
}

// 获取进程ID
DWORD GetProcessIdByName(const std::string& processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
                processId = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return processId;
}

// 检查是否为x64进程
bool IsProcessX64(HANDLE hProcess) {
    BOOL isWow64 = FALSE;
    if (IsWow64Process(hProcess, &isWow64)) {
        return !isWow64;
    }
    return false;
}






// 注入DLL到x64进程
bool InjectDLL(DWORD processId, const std::string& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        std::cerr << "无法打开目标进程。\n";
        return false;
    }

    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, nullptr, dllPath.size() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        std::cerr << "无法为DLL路径分配内存。\n";
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), dllPath.size() + 1, nullptr)) {
        std::cerr << "无法写入DLL路径到目标进程。\n";
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 获取LoadLibraryA的地址（针对x64）
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
    if (!pLoadLibraryA) {
        std::cerr << "无法获取LoadLibraryA地址。\n";
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // 创建远程线程并调用LoadLibraryA加载DLL
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibraryA), pRemoteMemory, 0, nullptr);
    if (!hThread) {
        std::cerr << "无法创建远程线程。\n";
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "核心加载成功！\n";
    SetCmdTitle("注入成功请关闭我  认证版权泪心QQ2254013571");

    return true;
}

// 等待目标进程并注入 DLL
void WaitForProcessAndInject(const std::string& processName, const std::string& dllPath) {
    while (!shouldStop) {
        DWORD processId = GetProcessIdByName(processName);
        if (processId != 0) {
            std::cout << "目标进程已找到，PID：" << processId << "\n";
            if (InjectDLL(processId, dllPath)) {
                std::cout << "DLL 注入成功！\n";
                SetConsoleTextColor(2);
                printf("请按回车键退出该程序该窗口...\n");
                // 注入成功后删除 DLL 文件
                if (DeleteFileA(dllPath.c_str())) {
                //    std::cout << "动态库文件 " << dllPath << " 已成功删除。\n";
                }
                else {
                 //   std::cerr << "删除动态库文件失败，错误代码：" << GetLastError() << "\n";
                }
            }
            else {
                std::cerr << "DLL 注入失败！\n";
            }
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}








// 检测文件是否存在
bool FileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

// 生成配置文件
void GenerateConfigFile(const std::string& filePath) {
    const std::string configContent =
        "# 配置文件示例\n"
        "#泪心寻找封包特征字节集\n"
        "leixinTargerHeaderHex=0x00 0x00 0x0A 0x11\n"
        "#泪心替换封包十六进制\n"
        "leixinReplacement=0xFF 0xFF 0xAA 0xBB\n"
        "#泪心替换位置 请输入十进制整数(替换)\n"
        "leixinposition=50\n"
        "#泪心封包递进位置数值(递进)\n"
        "leixindijin=77\n"
        "#当前确定是否启动发送替换方案\n"
        "send=0\n"
        "#请问是否启动接收替换方案\n"
        "recv=1";

    std::ofstream configFile(filePath);
    if (configFile.is_open()) {
        configFile << configContent;
        configFile.close();
        std::cout << "配置文件生成成功！文件路径为：" << filePath << std::endl;
    }
    else {
        std::cerr << "无法创建配置文件，请检查路径或权限。\n";
    }
}
 


 


int main() {
    SetConsoleTextColor(4);
    printf("泪心专享私用网络X64封包工具！！！\n");
    printf("泪心电报 https://t.me/TearGame  ！！！\n");
    printf("泪心QQ频道 点击链接加入腾讯频道【泪心and 独久 love频道】：https://pd.qq.com/s/estiees5z   ！！！\n");
    SetConsoleTextColor(7);
    std::string filePath = "tear.conf";
    std::string  charlespath = "Charles.ini";//根据花瓶配置文件判断是否在花瓶根目录下
    //我是否存在核心注入配置文件
    if (FileExists(filePath)) {
        std::cout << "配置文件已存在，无需重新生成。文件路径为您的花瓶根目录注入的tear.conf！！！！" << std::endl;
    }
    else {
        GenerateConfigFile(filePath);
    }
    //确认是否在花瓶根目录 通过文件名配置来定位
    if (FileExists(charlespath))
    {
        SetConsoleTextColor(6);
        printf("已经正确打开软件,正常使用泪心封包工具中!请注意配置文件在花瓶根目录下的tear.conf文件！\n");
    }
    else {
        SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // 设置为深红色
        printf("请把程序和配置文件复制粘贴到Charles根目录执行！否则无法启动！如:C:\\桌面\\Charles根目录下\n");
        SetConsoleTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // 恢复为默认颜色     
        system("pause");
        exit(0);
    }
 

    std::string targetProcessName = "Charles.exe";
    std::string dllPath = "tearwin.dll";

    //确认是否生成核心配置文件 当前只支持修改发送
    if (FileExists(dllPath))
    {
        SetConsoleTextColor(3);
        printf("泪心已经为您加载了核心配置文件,文件名为:tearwin.dll\n");
        SetConsoleTextColor(7);
    }
    else {
        writeFile("tearwin.dll", tearwin, sizeof(tearwin));
        SetConsoleTextColor(3);
        printf("泪心写入系统核心配置文件成功！\n");
        SetConsoleTextColor(7);
    }

    SetConsoleTextColor(7);
    std::cout << "启动注入器，等待目标进程...\n";
    SetCmdTitle("注入器正在等待目标进程");

    std::thread injectionThread(WaitForProcessAndInject, targetProcessName, dllPath);

    std::cin.get();  // 等待用户输入以退出
    shouldStop = true;
    injectionThread.join();

    std::cout << "注入器已退出。\n";
    system("pause");
    return 0;
}
