/*
  h - vec_h
  Created by Jackson Lago, Dez 15, 2021.
  Not released into the public domain.
*/
#ifndef vec_h
#define vec_h

#include <Arduino.h>

template<typename T>
class Vec2
{
    private:
    T data[2];

    public:
    Vec2(){};

    Vec2(T scalar)
    {
        data[0] = scalar;
        data[1] = scalar;
    }

    Vec2(T x0, T x1)
    {
        data[0] = x0;
        data[1] = x1;
    }

    Vec2(T* x)
    {
        data[0] = x[0];
        data[1] = x[1];
    }

    Vec2(const Vec2<T>& x)
    {
        data[0] = x[0];
        data[1] = x[1];
    }

    Vec2<T> operator*(const Vec2<T>& v)
    {
        Vec2<T> m;
        m[0] = data[0] * v[0];
        m[1] = data[1] * v[1];
        return m;
    }

    Vec2<T>& operator*=(const Vec2<T>& v)
    {
        data[0] *= v[0];
        data[1] *= v[1];
        return *this;
    }

   Vec2<T> operator+(const Vec2<T>& v)
    {
        Vec2<T> m;
        m[0] = data[0] + v[0];
        m[1] = data[1] + v[1];
        return m;
    }

    Vec2<T>& operator+=(const Vec2<T>& v)
    {
        data[0] += v[0];
        data[1] += v[1];
        return *this;
    }

    T operator[](uint16_t id) const
    {
        return data[id];
    }

    T& operator[](uint16_t id)
    {
        return data[id];
    }

    String str(void) const
    {
        return vec2str(*this);
    }

    String str(int decimals) const
    {
        return vec2str(*this, decimals);
    }

};

template<typename T>
String vec2str(const Vec2<T> &vec)
{
    return String("Vec2: [" + String(vec[0]) + ", " + String(vec[1]) + "]");
}

template<typename T>
String vec2str(const Vec2<T> &vec, int decimals)
{
    return String("Vec2: [" + String(vec[0], decimals) + ", " + String(vec[1], decimals) + "]");
}




template<typename T>
class Vec3
{
    private:
    T data[3];

    public:
    Vec3(){};

    Vec3(T scalar)
    {
        data[0] = scalar;
        data[1] = scalar;
        data[2] = scalar;
    }

    Vec3(T x0, T x1, T x2)
    {
        data[0] = x0;
        data[1] = x1;
        data[2] = x2;
    }

    Vec3(T* x)
    {
        data[0] = x[0];
        data[1] = x[1];
        data[2] = x[2];
    }

    Vec3(const Vec3<T>& x)
    {
        data[0] = x[0];
        data[1] = x[1];
        data[2] = x[2];
    }

    Vec3<T> operator*(const Vec3<T>& v)
    {
        Vec3<T> m;
        m[0] = data[0] * v[0];
        m[1] = data[1] * v[1];
        m[2] = data[2] * v[2];
        return m;
    }

    Vec3<T>& operator*=(const Vec3<T>& v)
    {
        data[0] *= v[0];
        data[1] *= v[1];
        data[2] *= v[2];
        return *this;
    }

   Vec3<T> operator+(const Vec3<T>& v)
    {
        Vec3<T> m;
        m[0] = data[0] + v[0];
        m[1] = data[1] + v[1];
        m[2] = data[2] + v[2];
        return m;
    }

    Vec3<T>& operator+=(const Vec3<T>& v)
    {
        data[0] += v[0];
        data[1] += v[1];
        data[2] += v[2];
        return *this;
    }

    T operator[](uint16_t id) const
    {
        return data[id];
    }

    T& operator[](uint16_t id)
    {
        return data[id];
    }

    String str(void) const
    {
        return vec3str(*this);
    }

    String str(int decimals) const
    {
        return vec3str(*this, decimals);
    }

};

template<typename T>
String vec3str(const Vec3<T> &vec)
{
    return String("Vec2: [" + String(vec[0]) + ", " + String(vec[1]) + ", " + String(vec[2]) + "]");
}

template<typename T>
String vec3str(const Vec3<T> &vec, int decimals)
{
    return String("Vec2: [" + String(vec[0], decimals) + ", " + String(vec[1], decimals) + ", " + String(vec[2], decimals) + "]");
}

#endif