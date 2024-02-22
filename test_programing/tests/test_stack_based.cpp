#include <iostream>
#include <vector>
#include <stack>

// 定义虚拟机指令集
enum OpCode {
    PUSH_INT,
    ADD,
    SUB,
    MUL,
    DIV,
    PRINT,
    HALT
};

// 定义虚拟机指令
struct Instruction {
    OpCode opCode;
    int operand;

    Instruction(OpCode code, int value = 0) : opCode(code), operand(value) {}
};

class VirtualMachine {
private:
    std::stack<int> stack;  // 虚拟机堆栈
    std::vector<Instruction> program;  // 存储虚拟机指令的程序

public:
    // 加载程序
    void loadProgram(const std::vector<Instruction>& programCode) {
        program = programCode;
    }

    // 执行程序
    void run() {
        for (size_t pc = 0; pc < program.size(); pc++) {
            const Instruction& currentInstruction = program[pc];
            switch (currentInstruction.opCode) {
                case PUSH_INT:
                    stack.push(currentInstruction.operand);
                    break;
                case ADD:
                    if (stack.size() < 2) {
                        std::cerr << "Error: Not enough operands for ADD operation." << std::endl;
                        return;
                    }
                    {
                        int a = stack.top();
                        stack.pop();
                        int b = stack.top();
                        stack.pop();
                        stack.push(a + b);
                    }
                    break;
                case SUB:
                    if (stack.size() < 2) {
                        std::cerr << "Error: Not enough operands for SUB operation." << std::endl;
                        return;
                    }
                    {
                        int a = stack.top();
                        stack.pop();
                        int b = stack.top();
                        stack.pop();
                        stack.push(b - a);
                    }
                    break;
                case MUL:
                    if (stack.size() < 2) {
                        std::cerr << "Error: Not enough operands for MUL operation." << std::endl;
                        return;
                    }
                    {
                        int a = stack.top();
                        stack.pop();
                        int b = stack.top();
                        stack.pop();
                        stack.push(a * b);
                    }
                    break;
                case DIV:
                    if (stack.size() < 2) {
                        std::cerr << "Error: Not enough operands for DIV operation." << std::endl;
                        return;
                    }
                    {
                        int a = stack.top();
                        stack.pop();
                        int b = stack.top();
                        stack.pop();
                        if (a == 0) {
                            std::cerr << "Error: Division by zero." << std::endl;
                            return;
                        }
                        stack.push(b / a);
                    }
                    break;
                case PRINT:
                    if (!stack.empty()) {
                        std::cout << "Result: " << stack.top() << std::endl;
                    }
                    break;
                case HALT:
                    return;  // 程序结束
            }
        }
    }
};

int main() {
    VirtualMachine vm;
    std::vector<Instruction> program = {
        {PUSH_INT, 5},
        {PUSH_INT, 3},
        {ADD},
        {PRINT},
        {HALT}
    };

    vm.loadProgram(program);
    vm.run();

    return 0;
}
