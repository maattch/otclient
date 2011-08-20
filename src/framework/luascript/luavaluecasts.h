#ifndef LUAVALUECASTS_H
#define LUAVALUECASTS_H

// this file is and must be included only from luainterface.h

#include "declarations.h"
#include <framework/otml/declarations.h>

// bool
void push_luavalue(bool b);
bool luavalue_cast(int index, bool& b);

// int
void push_luavalue(int i);
bool luavalue_cast(int index, int& i);

// double
void push_luavalue(double d);
bool luavalue_cast(int index, double& d);

// size_t
void push_luavalue(std::size_t s);
bool luavalue_cast(int index, std::size_t& s);

// string
void push_luavalue(const char* cstr);
void push_luavalue(const std::string& str);
bool luavalue_cast(int index, std::string& str);

// lua cpp function
void push_luavalue(const LuaCppFunction& func);

// color
void push_luavalue(const Color& color);
bool luavalue_cast(int index, Color& color);

// rect
void push_luavalue(const Rect& rect);
bool luavalue_cast(int index, Rect& rect);

// point
void push_luavalue(const Point& point);
bool luavalue_cast(int index, Point& point);

// size
void push_luavalue(const Size& size);
bool luavalue_cast(int index, Size& size);

// otml nodes
void push_luavalue(const OTMLNodePtr& node);

// enum
template<class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type
luavalue_cast(int index, T& myenum);

// LuaObject pointers
template<class T>
typename std::enable_if<std::is_base_of<LuaObject, typename T::element_type>::value, void>::type
push_luavalue(const T& obj);

bool luavalue_cast(int index, LuaObjectPtr& obj);

template<class T>
typename std::enable_if<std::is_base_of<LuaObject, T>::value, bool>::type
luavalue_cast(int index, std::shared_ptr<T>& ptr);

// std::function
template<typename Ret, typename... Args>
void push_luavalue(const std::function<Ret(Args...)>& func);

template<typename... Args>
bool luavalue_cast(int index, std::function<void(Args...)>& func);

template<typename Ret, typename... Args>
typename std::enable_if<!std::is_void<Ret>::value, bool>::type
luavalue_cast(int index, std::function<Ret(Args...)>& func);


// start definitions

#include "luaexception.h"
#include "luainterface.h"

template<class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type
luavalue_cast(int index, T& myenum) {
    return luavalue_cast(index, (int&)myenum);
}

template<class T>
typename std::enable_if<std::is_base_of<LuaObject, typename T::element_type>::value, void>::type
push_luavalue(const T& obj) {
    if(obj)
        return g_lua.pushObject(obj);
    return g_lua.pushNil();
}

template<class T>
typename std::enable_if<std::is_base_of<LuaObject, T>::value, bool>::type
luavalue_cast(int index, std::shared_ptr<T>& ptr) {
    LuaObjectPtr obj;
    if(!luavalue_cast(index, obj))
        return false;
    ptr = std::dynamic_pointer_cast<T>(obj);
    return true;
}

template<typename Ret, typename... Args>
void push_luavalue(const std::function<Ret(Args...)>& func) {
    if(func) {
        LuaCppFunction f = luabinder::bind_fun(func);
        g_lua.pushCppFunction(f);
    } else
        g_lua.pushNil();
}

template<typename... Args>
bool luavalue_cast(int index, std::function<void(Args...)>& func) {
    if(g_lua.isFunction(index)) {
        g_lua.pushValue(index);
        // weak references are used here, this means that the script must hold another reference
        // to this function, otherwise it will expire
        int funcWeakRef = g_lua.weakRef();
        func = [=](Args... args...) {
            // note that we must catch exceptions, because this lambda can be called from anywhere
            // and most of them won't catch exceptions (e.g. dispatcher)
            g_lua.getWeakRef(funcWeakRef);
            try {
                if(g_lua.isFunction()) {
                    g_lua.polymorphicPush(args...);
                    assert(g_lua.safeCall(sizeof...(Args)) == 0);
                } else {
                    throw LuaException("attempt to call an expired lua function from C++,"
                                       "did you forget to hold a reference for that function?", 0);
                }
            } catch(std::exception& e) {
                logError("lua function callback failed: ", e.what());
            }
        };
        return true;
    } else if(g_lua.isNil(index)) {
        func = std::function<void(Args...)>();
        return true;
    }
    return false;
}

template<typename Ret, typename... Args>
typename std::enable_if<!std::is_void<Ret>::value, bool>::type
luavalue_cast(int index, std::function<Ret(Args...)>& func) {
    if(g_lua.isFunction(index)) {
        g_lua.pushValue(index);
        // weak references are used here, this means that the script must hold another reference
        // to this function, otherwise it will expire
        int funcWeakRef = g_lua.weakRef();
        func = [=](Args... args...) -> Ret {
            // note that we must catch exceptions, because this lambda can be called from anywhere
            // and most of them won't catch exceptions (e.g. dispatcher)
            try {
                g_lua.getWeakRef(funcWeakRef);
                if(g_lua.isFunction()) {
                    g_lua.polymorphicPush(args...);
                    if(g_lua.safeCall(sizeof...(Args)) != 1)
                        throw LuaException("a function from lua didn't retrieve the expected number of results", 0);
                    return g_lua.polymorphicPop<Ret>();
                } else {
                    throw LuaException("attempt to call an expired lua function from C++,"
                                       "did you forget to hold a reference for that function?", 0);
                }
            } catch(std::exception& e) {
                logError("lua function callback failed: ", e.what());
            }
            return Ret();
        };
        return true;
    } else if(g_lua.isNil(index)) {
        func = std::function<Ret(Args...)>();
        return true;
    }
    return false;
}

#endif
