#include <iostream>
#include <cstring>

using namespace std;

// 简单的字符串类，演示拷贝与移动语义
class MyString
{
private:
    char *str; // 指向堆上分配的以'\0'结尾的字符数组
public:
    // 构造（接受 C 风格字符串）
    MyString(const char *ps = nullptr) : str(nullptr)
    {
        if (ps != nullptr && *ps != '\0')
        {
            size_t len = strlen(ps);
            str = new char[len + 1];
            strcpy(str, ps);
        }
        cout << "Create String Object: " << this << endl;
    }

    // 析构，释放资源
    ~MyString()
    {
        delete[] str;
        str = nullptr;
    }

    // 拷贝构造：深拷贝源对象的数据
    MyString(const MyString &s) : str(nullptr)
    {
        if (s.str)
        {
            size_t len = strlen(s.str);
            str = new char[len + 1];
            strcpy(str, s.str);
        }
        cout << "Copy Construct: " << this << " from " << &s << endl;
    }

    // 移动构造：接管源对象的资源，源对象置空
    MyString(MyString &&s) noexcept : str(s.str)
    {
        s.str = nullptr;
        cout << "Move Construct: " << this << " from " << &s << endl;
    }

    // 拷贝赋值：注意自赋值，采用先分配再释放的异常安全策略
    MyString &operator=(const MyString &s)
    {
        if (this == &s)
            return *this;

        char *newstr = nullptr;
        if (s.str)
        {
            size_t len = strlen(s.str);
            newstr = new char[len + 1];
            strcpy(newstr, s.str);
        }

        delete[] str;
        str = newstr;
        cout << "Copy Assign: " << this << " from " << &s << endl;
        return *this;
    }

    // 移动赋值：释放自身资源并接管对方资源
    MyString &operator=(MyString &&s) noexcept
    {
        if (this != &s)
        {
            delete[] str;
            str = s.str;
            s.str = nullptr;
        }
        cout << "Move Assign: " << this << " from " << &s << endl;
        return *this;
    }

    // 打印当前字符串内容
    void Print() const
    {
        if (str != nullptr)
        {
            cout << "str: " << str << endl;
        }
        else
        {
            cout << "str: empty string" << endl;
        }
    }
//类内，对象+对象
MyString operator+(const MyString& other)const
{
    size_t l1=strlen(str);
    size_t l2=strlen(other.str);
    char* p=new char[l1+l2+1];
    strcpy(p,str);
    strcat(p,other.str);
    MyString temp(p);
    delete[]p;
    return temp;
}
//类内：对象+常量字符串
MyString operator+(const char* ps)const
{
    return *this + MyString(ps);
}

};
//类外全局函数，放在类定义外部
MyString operator+(const char* ps, const MyString& s)
{
    return s+ps;
}

int main()
{

     MyString s1("yhping"),s2("hello");
     MyString s3;
     s3 = s1 + s2;
     s3.Print();
     s3 = s1 + "hello";
     s3.Print();
     s3 = "hello" + s1;
     s3.Print();
}

