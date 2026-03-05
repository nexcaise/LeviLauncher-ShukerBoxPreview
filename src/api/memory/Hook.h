#pragma once
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <unistd.h>

#include "api/Macro.h"
#include "api/memory/Memory.h"
#include <type_traits>

#define LOG(...)

namespace LL_vhook {

    struct MapRegion {
        uintptr_t start;
        uintptr_t end;
        bool readable;
        bool executable;
    };

    inline std::vector<MapRegion> getMinecraftRegions() {
        std::vector<MapRegion> regions;
        std::ifstream maps("/proc/self/maps");
        std::string line;

        while (std::getline(maps, line)) {
            if (line.find("libminecraftpe.so") == std::string::npos)
                continue;

            uintptr_t start, end;
            char perms[5];

            if (sscanf(line.c_str(), "%lx-%lx %4s", &start, &end, perms) != 3)
                continue;

            MapRegion r{};
            r.start = start;
            r.end = end;
            r.readable = perms[0] == 'r';
            r.executable = perms[2] == 'x';

            regions.push_back(r);
        }

        return regions;
    }

    inline bool hook(
        const char* typeinfoName,
        int vtableIndex,
        void* hookFunc,
        void** orig
    ) {
        auto regions = getMinecraftRegions();

        size_t nameLen = strlen(typeinfoName);
        uintptr_t typeinfoNameAddr = 0;

        for (auto& r : regions) {
            if (!r.readable) continue;

            for (uintptr_t a = r.start; a < r.end - nameLen; a++) {
                if (!memcmp((void*)a, typeinfoName, nameLen)) {
                    typeinfoNameAddr = a;
                    break;
                }
            }
            if (typeinfoNameAddr) break;
        }

        if (!typeinfoNameAddr) return false;

        uintptr_t typeinfoAddr = 0;

        for (auto& r : regions) {
            if (!r.readable) continue;

            for (uintptr_t a = r.start; a < r.end; a += sizeof(void*)) {
                if (*(uintptr_t*)a == typeinfoNameAddr) {
                    typeinfoAddr = a - sizeof(void*);
                    break;
                }
            }
            if (typeinfoAddr) break;
        }

        if (!typeinfoAddr) return false;

        uintptr_t vtableAddr = 0;

        for (auto& r : regions) {
            if (!r.readable) continue;

            for (uintptr_t a = r.start; a < r.end; a += sizeof(void*)) {
                if (*(uintptr_t*)a == typeinfoAddr) {
                    vtableAddr = a + sizeof(void*);
                    break;
                }
            }
            if (vtableAddr) break;
        }

        if (!vtableAddr) return false;

        uintptr_t* slot =
            (uintptr_t*)(vtableAddr + vtableIndex * sizeof(void*));

        *orig = (void*)(*slot);

        size_t pageSize = sysconf(_SC_PAGESIZE);
        uintptr_t page = (uintptr_t)slot & ~(pageSize - 1);

        mprotect((void*)page, pageSize, PROT_READ | PROT_WRITE);
        *slot = (uintptr_t)hookFunc;
        mprotect((void*)page, pageSize, PROT_READ);

        return true;
    }
}

namespace memory {

using FuncPtr = void *;

void unhookAll();

template <typename T> struct IsConstMemberFun : std::false_type {};

template <typename T, typename Ret, typename... Args>
struct IsConstMemberFun<Ret (T::*)(Args...) const> : std::true_type {};

template <typename T>
inline constexpr bool IsConstMemberFunV = IsConstMemberFun<T>::value;

template <typename T> struct AddConstAtMemberFun {
  using type = T;
};

template <typename T, typename Ret, typename... Args>
struct AddConstAtMemberFun<Ret (T::*)(Args...)> {
  using type = Ret (T::*)(Args...) const;
};

template <typename T>
using AddConstAtMemberFunT = typename AddConstAtMemberFun<T>::type;

template <typename T, typename U>
using AddConstAtMemberFunIfOriginIs =
    std::conditional_t<IsConstMemberFunV<U>, AddConstAtMemberFunT<T>, T>;

/**
 * @brief Hook priority enum.
 * @details The lower priority, the hook will be executed earlier
 */
enum class HookPriority : int {
  Lowest = 0,
  Low = 100,
  Normal = 200,
  High = 300,
  Highest = 400,
};

int hook(FuncPtr target, FuncPtr detour, FuncPtr *originalFunc,
         HookPriority priority, bool suspendThreads = true);

bool unhook(FuncPtr target, FuncPtr detour, bool suspendThreads = true);

/**
 * @brief Get the pointer of a function by identifier.
 *
 * @param identifier symbol or signature
 * @return FuncPtr
 */
FuncPtr resolveIdentifier(char const *identifier);
FuncPtr resolveIdentifier(std::initializer_list<const char *> identifiers);

template <typename T>
concept FuncPtrType = std::is_function_v<std::remove_pointer_t<T>> ||
                      std::is_member_function_pointer_v<T>;

template <typename T>
  requires(FuncPtrType<T> || std::is_same_v<T, uintptr_t>)
constexpr FuncPtr resolveIdentifier(T identifier) {
  return toFuncPtr(identifier);
}

// redirect to resolveIdentifier(char const*)
template <typename T>
constexpr FuncPtr resolveIdentifier(char const *identifier) {
  return resolveIdentifier(identifier);
}

// redirect to resolveIdentifier(uintptr_t)
template <typename T> constexpr FuncPtr resolveIdentifier(uintptr_t address) {
  return resolveIdentifier(address);
}

// redirect to resolveIdentifier(FuncPtr)
template <typename T> constexpr FuncPtr resolveIdentifier(FuncPtr address) {
  return address;
}

template <typename T>
constexpr FuncPtr
resolveIdentifier(std::initializer_list<const char *> identifiers) {
  return resolveIdentifier(identifiers);
}

template <typename T> struct HookAutoRegister {
  HookAutoRegister() { T::hook(); }
  ~HookAutoRegister() { T::unhook(); }
  static int hook() { return T::hook(); }
  static bool unhook() { return T::unhook(); }
};

} // namespace memory

#define HOOK_IMPL(REGISTER, FUNC_PTR, STATIC, CALL, DEF_TYPE, TYPE, PRIORITY,  \
                  IDENTIFIER, RET_TYPE, ...)                                   \
  struct DEF_TYPE TYPE {                                                       \
    using FuncPtr = ::memory::FuncPtr;                                         \
    using HookPriority = ::memory::HookPriority;                               \
    using OriginFuncType = ::memory::AddConstAtMemberFunIfOriginIs<            \
        RET_TYPE FUNC_PTR(__VA_ARGS__), decltype(IDENTIFIER)>;                 \
                                                                               \
    inline static FuncPtr target{};                                            \
    inline static OriginFuncType originFunc{};                                 \
                                                                               \
    template <typename... Args> STATIC RET_TYPE origin(Args &&...params) {     \
      return CALL(std::forward<Args>(params)...);                              \
    }                                                                          \
                                                                               \
    STATIC RET_TYPE detour(__VA_ARGS__);                                       \
                                                                               \
    static int hook() {                                                        \
      target = memory::resolveIdentifier<OriginFuncType>(IDENTIFIER);          \
      if (target == nullptr) {                                                 \
        return -1;                                                             \
      }                                                                        \
      return memory::hook(target, memory::toFuncPtr(&DEF_TYPE::detour),        \
                          reinterpret_cast<FuncPtr *>(&originFunc), PRIORITY); \
    }                                                                          \
                                                                               \
    static bool unhook() {                                                     \
      return memory::unhook(target, memory::toFuncPtr(&DEF_TYPE::detour));     \
    }                                                                          \
  };                                                                           \
  REGISTER;                                                                    \
  RET_TYPE DEF_TYPE::detour(__VA_ARGS__)

#define LL_AUTO_REG_HOOK_IMPL(FUNC_PTR, STATIC, CALL, DEF_TYPE, ...)          \
  VA_EXPAND(HOOK_IMPL(                                                         \
      inline memory::HookAutoRegister<DEF_TYPE> DEF_TYPE##AutoRegister,        \
      FUNC_PTR, STATIC, CALL, DEF_TYPE, __VA_ARGS__))

#define LL_MANUAL_REG_HOOK_IMPL(...) VA_EXPAND(HOOK_IMPL(, __VA_ARGS__))

#define LL_STATIC_HOOK_IMPL(...)                                              \
  VA_EXPAND(LL_MANUAL_REG_HOOK_IMPL((*), static, originFunc, __VA_ARGS__))

#define LL_AUTO_STATIC_HOOK_IMPL(...)                                         \
  VA_EXPAND(LL_AUTO_REG_HOOK_IMPL((*), static, originFunc, __VA_ARGS__))

#define LL_INSTANCE_HOOK_IMPL(DEF_TYPE, ...)                                  \
  VA_EXPAND(LL_MANUAL_REG_HOOK_IMPL((DEF_TYPE::*), , (this->*originFunc),     \
                                     DEF_TYPE, __VA_ARGS__))

#define LL_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE, ...)                             \
  VA_EXPAND(LL_AUTO_REG_HOOK_IMPL((DEF_TYPE::*), , (this->*originFunc),       \
                                   DEF_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a typed static function.
 * @param DefType The name of the hook definition.
 * @param type The type of the function.
 * @param priority The priority of the hook.
 * @param identifier The identifier of the hook. It can be a function symbol,
 * address or a signature.
 * @param Ret The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DefType::hook() and
 * DefType::unhook().
 */
#define LL_TYPED_STATIC_HOOK(DefType, type, priority, identifier, Ret, ...)   \
  VA_EXPAND(LL_STATIC_HOOK_IMPL(DefType,                                      \
                                 : public type, priority, identifier, Ret,     \
                                   __VA_ARGS__))

/**
 * @brief Register a hook for a static function.
 * @param DefType The name of the hook definition.
 * @param priority The priority of the hook.
 * @param identifier The identifier of the hook. It can be a function symbol,
 * address or a signature.
 * @param Ret The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DefType::hook() and
 * DefType::unhook().
 */
#define LL_STATIC_HOOK(DefType, priority, identifier, Ret, ...)               \
  VA_EXPAND(                                                                   \
      LL_STATIC_HOOK_IMPL(DefType, , priority, identifier, Ret, __VA_ARGS__))

/**
 * @brief Register a hook for a typed static function.
 * @details The hook will be automatically registered and unregistered.
 * @see TYPED_STATIC_HOOK for usage.
 */
#define LL_AUTO_TYPED_STATIC_HOOK(DefType, type, priority, identifier, Ret,   \
                                   ...)                                        \
  VA_EXPAND(LL_AUTO_STATIC_HOOK_IMPL(DefType,                                 \
                                      : public type, priority, identifier,     \
                                        Ret, __VA_ARGS__))

/**
 * @brief Register a hook for a static function.
 * @details The hook will be automatically registered and unregistered.
 * @see STATIC_HOOK for usage.
 */
#define LL_AUTO_STATIC_HOOK(DefType, priority, identifier, Ret, ...)          \
  VA_EXPAND(LL_AUTO_STATIC_HOOK_IMPL(DefType, , priority, identifier, Ret,    \
                                      __VA_ARGS__))

/**
 * @brief Register a hook for a typed instance function.
 * @param DEF_TYPE The name of the hook definition.
 * @param PRIORITY memory::HookPriority The priority of the hook.
 * @param TYPE The type which the function belongs to.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer,
 * symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and
 * DEF_TYPE::unhook().
 */
#define LL_TYPED_HOOK(DEF_TYPE, PRIORITY, TYPE, IDENTIFIER, RET_TYPE, ...)    \
  VA_EXPAND(LL_INSTANCE_HOOK_IMPL(DEF_TYPE,                                   \
                                   : public TYPE, PRIORITY, IDENTIFIER,        \
                                     RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a instance function.
 * @param DEF_TYPE The name of the hook definition.
 * @param PRIORITY memory::HookPriority The priority of the hook.
 * @param IDENTIFIER The identifier of the hook. It can be a function pointer,
 * symbol, address or a signature.
 * @param RET_TYPE The return type of the hook.
 * @param ... The parameters of the hook.
 *
 * @note register or unregister by calling DEF_TYPE::hook() and
 * DEF_TYPE::unhook().
 */
#define LL_INSTANCE_HOOK(DEF_TYPE, PRIORITY, IDENTIFIER, RET_TYPE, ...)       \
  VA_EXPAND(LL_INSTANCE_HOOK_IMPL(DEF_TYPE, , PRIORITY, IDENTIFIER, RET_TYPE, \
                                   __VA_ARGS__))

/**
 * @brief Register a hook for a typed instance function.
 * @details The hook will be automatically registered and unregistered.
 * @see TYPED_HOOK for usage.
 */
#define LL_AUTO_TYPED_INSTANCE_HOOK(DEF_TYPE, PRIORITY, TYPE, IDENTIFIER,     \
                                     RET_TYPE, ...)                            \
  VA_EXPAND(LL_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE,                              \
                                        : public TYPE, PRIORITY, IDENTIFIER,   \
                                          RET_TYPE, __VA_ARGS__))

/**
 * @brief Register a hook for a instance function.
 * @details The hook will be automatically registered and unregistered.
 * @see HOOK for usage.
 */
#define LL_AUTO_INSTANCE_HOOK(DEF_TYPE, PRIORITY, IDENTIFIER, RET_TYPE, ...)  \
  VA_EXPAND(LL_AUTO_INSTANCE_HOOK_IMPL(DEF_TYPE, , PRIORITY, IDENTIFIER,      \
                                        RET_TYPE, __VA_ARGS__))

#define LL_VTABLE_HOOK(NAME, TYPEINFO, INDEX, RET, ...)      \
    struct NAME {                                             \
        using FuncType = RET(*)(__VA_ARGS__);                  \
        static inline FuncType origin = nullptr;               \
        static RET impl(__VA_ARGS__);                          \
        static bool hook() {                                \
            return LL_vhook::hook(                            \
                TYPEINFO,                                      \
                INDEX,                                         \
                (void*)&impl,                                  \
                (void**)&origin                                \
            );                                                 \
        }                                                      \
    };                                                         \
    RET NAME::impl(__VA_ARGS__)

#define LL_AUTO_VTABLE_HOOK(NAME, TYPEINFO, INDEX, RET, ...) \
    using NAME##_t = RET(*)(__VA_ARGS__);                      \
    static NAME##_t NAME##_origin = nullptr;                   \
    static RET NAME##_impl(__VA_ARGS__);                       \
    struct NAME##_Auto {                                       \
        NAME##_Auto() {                                        \
            LL_vhook::hook(                                   \
                TYPEINFO,                                      \
                INDEX,                                         \
                (void*)&NAME##_impl,                           \
                (void**)&NAME##_origin                         \
            );                                                 \
        }                                                      \
    };                                                         \
    static NAME##_Auto NAME##_auto;                            \
    static RET NAME##_impl(__VA_ARGS__)
