#pragma once
#include<fstream>
#include<iostream>
#include<string>
#include <windows.h>
#include <tlhelp32.h>
 
#include <thread>
#include <atomic>
 

#include <vector>
bool writeFile(const std::string& filename, const unsigned char* data, size_t size);

