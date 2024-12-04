#include <iostream>
#include <memory>

using namespace std;

int main()
{
    weak_ptr<int> wp1, wp2;
    shared_ptr<int> sp3;
    {
        shared_ptr<int> sp1(new int);
        shared_ptr<int> sp2(new int);
        sp3 = sp2;
        wp1 = sp1; // 用shared_ptr为weak_ptr赋值
        wp2 = sp2;
        *sp1 = 1;
        *sp2 = 2;
    }
    // 这里sp1, sp2已经被释放，sp3仍然存在，尝试通过wp1，wp2访问对象

    if (auto sp1 = wp1.lock()) {
        cout << "sp1: Succeed! *sp1 = " << *sp1 << endl;
    } else {
        // 理论上如果对象已经被销毁，lock() 方法会返回一个空的 shared_ptr
        // 此时无法访问对象
        cout << "sp1: Failed!" << endl;
    }

    if (auto sp2 = wp2.lock()) {
        cout << "sp2: Succeed! *sp2 = " << *sp2 << endl;
    } else {
        cout << "sp2: Failed!" << endl;
    }

    return 0;
}
