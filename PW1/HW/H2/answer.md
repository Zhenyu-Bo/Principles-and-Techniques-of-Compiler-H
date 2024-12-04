**问题1**

1. `-nostdinc` 选项用于禁用标准系统头文件目录的搜索。

2. 使用`gcc -v -x c -E /dev/null`可以查得 EduCoder 平台上 `gcc` C程序默认的头文件查找路径如下：

   ```
   /usr/lib/gcc/x86_64-linux-gnu/5/include
   /usr/local/include
   /usr/lib/gcc/x86_64-linux-gnu/5/include-fixed
   /usr/include/x86_64-linux-gnu
   /usr/include
   ```

3. 可以通过使用 `-I` 选项手动指定标准头文件路径。对于sample.c，可以使用以下命令来编译：

   ```shell
   gcc -nostdinc \
       -I/usr/lib/gcc/x86_64-linux-gnu/5/include \
       -I/usr/local/include \
       -I/usr/lib/gcc/x86_64-linux-gnu/5/include-fixed \
       -I/usr/include/x86_64-linux-gnu \
       -I/usr/include \
       sample.c -o sample
   ```

   这是因为 `-I` 选项手动指定了头文件路径，覆盖了 `-nostdinc` 的影响，使得编译器能够找到并使用指定的头文件。

**问题2**

1. `-nostdlib` 选项用于禁用标准系统库和启动文件的链接。

2. 使用`gcc -v`命令编译`sample.c`可以查看 GCC 在链接阶段使用的详细信息，包括默认链接的库。根据输出可得EduCoder 平台上 `gcc` C程序默认链接的库有标准C库`libc`(-lc), GCC支持库`libgcc`(-lgcc)和`libgcc_s`(-lgcc_s)。

3. 可以手动指定所有必要的启动文件和库，使用`-lc`, `-lgcc`, `-lgcc_s`选项手动添加上面查找得到的需要链接的库，并直接指定启动文件。启动文件也可通过`gcc -v`查找得到。

   这是因为通过手动添加启动文件和链接的库，链接器可以找到并使用这些文件，覆盖了 `-nostdlib` 的影响。
