# step3

## dynamic_cast

1. 作用是统计`values`中的 UnaryInst 和 BinaryInst 的数量，如果有既不是 UnaryInst 也不是 BinaryInst 的 Value 对象，则报错并终止程序。
   这段代码的`if`和`else if`中对`dynamic_cast`的使用用到了RTTI机制，分别尝试将 Value 对象转换为 UnaryInst 类型和 BinaryInst 类型，如果转换成功，就将相应类型的计数加1。
2. 如果没有RTTI机制，可以 Instruction 类中添加一个枚举类型`InstType`，在类的构造函数中为`InstType`设置相应的值来标识类型，然后可以在`BasicBlock::print`函数中根据 Value 对象的`InstType`的值来进行类型判断。
3. 可以在 Value 或 Instruction 类中添加一个纯虚函数`getType`用于返回类型，每个子类都实现该函数返回唯一的类型标识符。由于 BasicBlock 中定义的`values`数组是 Value 类型的，所以我将`getType`函数定义在 Value 类中，所有继承自 Value 的类都必须实现该函数，确保了所有派生类都具有统一的类型识别接口。也可以使用已定义的`getName`函数来获取类的名称以判断类型，前提是必须在每个派生类的构造函数中调用`setName`函数为其定义唯一的名称，相比之下，纯虚函数要求所有派生类必须实现，更为安全。

## typeid

1. 程序的输出为

   ```
   P5Value
   10BasicBlock
   P11Instruction
   10BinaryInst
   ```

   解释：因为`v`和`inst`分别声明为`Value *`和`Instruction *`类型的指针，所以它们的 typeid 即为相应类型的指针`P5Value`，`P11Instruction`。但是`v`指向 Value 的派生类 BasicBlock ，所以对它进行解引用后得到的是 BasicBlock 类型的对象，所以对应的 typeid 为`10BasicBlock`。同理，`inst`指向 Instruction 的派生类 BinaryInst ，所以对它进行解引用后得到的是 BinaryInst 类型的对象，所以对应的typeid为`10BinaryInst`。

2. 程序的输出是
   
   ```
   P5Value
   10BasicBlock
   P11Instruction
   11Instruction
   ```

   解释：改变 Instruction 不影响 Value 和 BasicBlock ，所以`typeid(v)`和`tyid(*v)`不变，解释同上一问。`inst`仍然声明为`Instruction *`类型，所以对应的 typeid 还是`P11Instruction`。变化的是`typeid(*inst)`，对`inst`解引用得到的是 BinaryInst 对象。但是由于 Instruction 类没有虚函数，所以`typeid(*inst)` 将返回指针的静态类型，即 Instruction。所以输出将是 Instruction 类型的名称`11Instruction`。
