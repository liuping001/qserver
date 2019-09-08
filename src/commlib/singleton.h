/**
 * 可以这样使用
 * class A : public S<A> {
 *  ...
 * }
 * A::get()
 * 或
 * class A {
 *  ...
 * };
 * S<A>::get();
 */
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

protected:
    S() {}
public:
    S(S const&)               = delete;
    void operator=(S const&)  = delete;
};