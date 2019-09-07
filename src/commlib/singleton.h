#pragma once

template <class T>
class S
{
public:
    static T& get()
    {
        static T instance;
        return   instance;
    }
private:
    S() {}
public:
    S(S const&)               = delete;
    void operator=(S const&)  = delete;
};