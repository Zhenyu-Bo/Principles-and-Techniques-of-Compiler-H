#include <iostream>
#include <typeinfo>
using namespace std;

class Base {
public:
    virtual void show() {
        cout << "Base class" << endl;
    }
};

class Derived : public Base {
public:
    void show() {
        cout << "Derived class" << endl;
    }
};

template <typename T>
void printType(const char *name, T t) {
    cout << "Type of " << name << ": " << typeid(t).name() << endl;
}
int main()
{
    int a;
    int *pa = &a;
    float b;
    float *pb = &b;
    char c;
    char *pc = &c;
    string d;
    string *pd = &d;

    Base baseObj; // 基类对象
    Derived derivedObj; // 派生类对象

    Base *base_ptr = &baseObj; // 基类指针
    Derived *derived_ptr = &derivedObj; // 派生类指针
    Base *ptr = &derivedObj; // 声明为基类指针，但指向派生类

    printType<int>("int", a);
    printType<int*>("int*", pa);
    printType<float>("float", b);
    printType<float*>("float*", pb);
    printType<char>("char", c);
    printType<char*>("char*", pc);
    printType<string>("string", d);
    printType<string*>("string*", pd);
    printType<Base>("baseObj", baseObj); // 基类对象
    printType<Derived>("derivedObj", derivedObj); // 派生类对象
    printType<Base*>("base_ptr", base_ptr); // 基类指针
    printType<Base>("*base_ptr", *base_ptr); // 基类指针指向的对象
    printType<Derived*>("derived_ptr", derived_ptr); // 派生类指针
    printType<Derived>("*derived_ptr", *derived_ptr); // 派生类指针指向的对象
    printType<Base*>("ptr", ptr); // 指向派生类的基类指针
    printType<Base>("*ptr", *ptr); // 指向派生类的基类指针指向的对象

    return 0;
}
