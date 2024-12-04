#include <iostream>
#include <complex>

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
private:
    int d;
};

int main()
{
    // static_cast
    short s = 1;
    int i = static_cast<int>(s); // 宽转换
    char c = static_cast<char>(i); // 窄转换
    int *pi = static_cast<int*>( malloc(10 * sizeof(int)) );  //将void指针转换为具体类型指针
    void *pv = static_cast<void*>(pi);  //将具体类型指针，转换为void指针
    double d = 1.0;
    complex<double> co = static_cast<complex<double>>(d); // 调用转换构造函数转换
    double realPart = static_cast<double>(co.real()); // 调用转换构造函数转换

    // dynamic_cast
    Base *p1 = new Derived; // 声明为基类指针，指向派生类对象
    Base *p2 = new Base; // 声明为基类指针，指向基类对象
    Derived *ptr = new Derived; // 派生类指针
    Base *ptrb = dynamic_cast<Base *>(ptr); // 向上转型，派生类指针转换为基类指针
    Base *pb = dynamic_cast<Base *>(p1); // 指向派生类对象的基类指针转换为基类指针
    Derived *pd1 = dynamic_cast<Derived *>(p1); // 向下转型, 指向派生类对象的基类指针转换为派生类指针
    Derived *pd2 = dynamic_cast<Derived *>(p2); // 向下转型, 指向基类对象的基类指针转换为派生类指针
    // 向下转型可能失败，对指针使用dynamic_cast的情况失败时返回空指针
    if (!pd1) {
        cout << "p1 to derived ptr dynamic_cast failed" << endl;
    }
    if (!pd2) {
        cout << "p2 to derived ptr dynamic_cast failed" << endl;
    }

    return 0;
}
