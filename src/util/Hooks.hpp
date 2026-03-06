#pragma once
#include <cstdint>

#include <pl/Signature.h>
#include <pl/Hook.h>
#include "util/Log.hpp"

#include <fstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>

uintptr_t scan(const char* sig);
bool VHOOK(const char*, int, void*, void**);
bool HOOK(const char*, void*, void**);