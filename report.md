# 图灵机模拟器 实验报告

## 实验方法设计

模拟器主要分为`Parser`和`Simulator`两部分，其中`Parser`负责解析输入的`*.tm`文件，记录其中定义的图灵机状态与转移函数；`Simulator`负责根据解析的`*.tm`文件以及输入的`input`模拟图灵机的运行过程。

### TuringState

`TuringState`是定义图灵机七元组
$TM = (Q,\Sigma,\Gamma,\delta,q_0,B,F)$
的数据结构抽象。其简要定义如下：

```c++
struct TuringState {
  SymbolSet symbols;
  StatesSet states;
  SymbolSet tapeSymbols;
  State initialState;
  Symbol blankSymbol;
  StatesSet finalStates;
  Size tapeCount;
  Transitions transitions;
};
```

需要说明的是$\delta$在`TuringState`中被定义为`Transition`的集合，以更方便进行模拟；因为要求实现多带图灵机，故额外`tapeCount`代表图灵机的纸带数量。其余状态的定义与数学形式基本一致。

### Parser

Parser解析过程如下：

1. 按行读取`tm`文件。根据每行的开头字符确定该行定义的状态（如`#Q`开头代表了图灵机的内部状态），我们根据相应的正则表达式对其进行解析，并存储到容器之中。如果开头无特殊标志，则解析为转移规则，该部分的解析方法会在下面详细说明。
2. 存储：我们将所有解析后的图灵机特性存入`TuringState`对象中，该类是联系`Parser`与`Simulator`的桥梁。
3. 当`Parser`完成任务后，会用得到的`TuringState`与解析的输入串生成`Simulator`，以开始模拟的过程。

### Transition解析

`Transition`转移分为确定性转移和非确定性转移。一条转移语句的数据结构如下：
```c++
struct Transition {
  State curr;    // 图灵机当前状态
  Symbols input; // 图灵机接受的符号

  State next;     // 图灵机转移后的状态
  Symbols output; // 图灵机写入符号
  Moves moves;    // 图灵机移动方向
};
```
因为实现了多带图灵机，故读取/写入符号与移动方向均为复数，代表 对应多条纸带。

在实际模拟过程中不支持非确定性转移，因此在解析过程我们使用了广度优先搜索的方法将非确定性转移转换为多条确定性转移，算法如下：

> 详细的实现位于[Machine.h (L179)](turing-project/Machine.h)

1. 维护`R`, `Q`，分别代表输出的确定转移语句的集合和代转换的非确定转移语句集合。
2. 向`Q`添加`*this`（即语句本身）
3. 当`Q`不为空时
   1. `front <- Q.deque`
   2. 如果`front`为确定性语句，将`front`加入`R`，继续循环。
   3. 对于每个非确定性转移，将`*`替换为所有可能的输入符号（除了空格），并将生成的新的转移加入`Q`。需要注意如果`input,output`同时含有`*`，需要保持二者替换的一致性。
4. 输出`R`

为了方便后续转移，所有转换后`Transition`会以`(curr, input)`作为`key`，`(next, output, moves)`作为`value`存入`map`。

### Simulator

`Simulator`的工作类似于CPU，其工作流程如下：
1. 根据当前`head`指向的符号与输入符号取得`Transition`
2. 执行`Transition`，包括写入符号，移动`head`指针
3. 修改计数器`step`，并输出对应提示（verbose模式）
4. 重复上述过程直到接受或无法取到`Transition`

因为图灵机是否能在某输入下停机是不可判定的，因此我们不保证`Simulator`一定会停止。

### Tape

`Tape`是对图灵机纸带的抽象，也是模拟器最核心的部分。

因为`C++`顺序容器不允许负数下标，因此我们设置一个`offset`函数获取纸带的下标对应的实际容器下标。

```c++
struct Tape {
  auto offset(Position pos) const -> Position { return pos - start(); }
  auto head() const -> Position { return _head; }
  auto start() const -> Position { return _start; }
  auto stop() const -> Position { return _start + tape.size(); }
};
```

### 其它模块

#### 字符串工具类

因为`C++`对字符串支持较差，因此我们额外实现了字符串的`format`, `join`, `split`等函数用于实现了解析和`Logger`输出。同时实现了将变量转换为字符串的`toString`函数，用于方便地输出当前的纸带状态。

#### Logger

因为部分输出只在`verbose`下输出，部分只在非`verbose`模式下输出，部分在两种情况下均输出，因此我们实现了`Logger`用来控制输出。

```c++
struct Logger {
  auto setVerbose(bool verbose) -> void;
  auto verbose(Level level, std::string_view fmt, auto &&...args) const -> void;
  auto noVerbose(Level level, std::string_view fmt, auto &&...args) const -> void;
};
```

## 实验完成情况

### 基础功能

实现的方法在上一节已经详细说明。

主要难点：非确定转移语句的处理。
解决方法：在上一节已经详细说明。

### verbose实现

我们为`Tapes`实现了`toString`函数，以得到当前纸带状态。根据已经实现的`Logger`，将状态格式化后输出。

主要难点：对齐输出纸带编号。
解决方法：根据纸带数目以及当前的纸带序号的长度，计算需要的缩进距离。

```c++
static auto getLength(int n) -> int {
  if (n == 0) {
    return 1;
  }
  auto length = 0;
  while (n != 0) {
    n /= 10;
    length++;
  }
  return length;
}

this->indent = std::string(getLength(state.tapeCount) - getLength(index), ' ');
```

### 多带图灵机程序设计

#### 循环右移 (case1)

1. 复制第一条纸带的内容到第二条
2. 根据最后符号转移到对应的状态，如果以0结束，则转移到回填0的状态；否则转移到回填1的状态；
3. 左移第二条纸带的读写头，错开一个单位。
4. 左移两条纸带并写入，直到第一条纸带左移到头。
5. 根据当前状态回填0或1。

#### 长度为平方数的字符串 (case2)

基本原理：`n^2 = 1+3+5+...+2n-1`
1. 对于纸带1，一直右移。
2. 对于纸带2，初始时长度为1，每次读到头时，长度增加2，即1-3-5-...
3. 如果纸带1读完，纸带2读头处于纸带其中一端，则接受；否则拒绝。
4. 进入接受或拒绝状态，在纸带1输出true或者false。

## 总结感想

1. 更深刻地理解了图灵机的运行原理
2. 了解了如何利用图灵机实现算法。
3. 更熟练地使用`C++`设计程序。

## 对课程和实验的意见与建议

建议：
1. 实验说明可以更加详细一些。如输出错误类型限定、格式说明、非确定转移与其它转移冲突时的处理方法不够明确，容易引起歧义。
2. 实验提交方法可以使用OJ，以方便进行提交版本的区分以及基本样例能否通过。
