#include <iostream>
#include <memory>

using namespace std;
class Test
{
private:
    /* data */
public:
    Test() {
        cout << "Test created" << endl;
    };
    ~Test() {
        cout << "Test deleted" << endl;
    };
};

void func(auto_ptr<Test> p) {
    // 接受一个auto_ptr类型的参数，获得所有权，函数返回后p对象会被销毁
    return;
}

int main()
{
    auto_ptr<Test> p(new Test());
    func(p); // 调用func函数会将p对象的所有权转移给函数内部的auto_ptr对象，函数返回后p对象会被销毁
    // 检查p是否为空，这里直接使用if判断，也可以使用assert
    if (p.get() == nullptr) {
        cout << "It is NULL now" << endl;
    }
    return 0;
}
