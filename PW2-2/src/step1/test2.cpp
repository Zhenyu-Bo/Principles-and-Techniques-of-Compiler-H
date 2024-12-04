#include <iostream>
#include <memory>

using namespace std;

int main()
{
    auto_ptr<int[]> p(new int[10]);
    // 由于 auto_ptr 使用非数组的 delete 来删除其指向的元素
    // 这里会导致未定义行为，并且不会正确释放数组内存，导致内存泄漏
    return 0;
}