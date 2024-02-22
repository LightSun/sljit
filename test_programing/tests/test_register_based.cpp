#include <iostream>
#include <vector>

enum OpCode {
    // 寄存器操作
    MOV_REG_REG,  // 寄存器到寄存器复制
    MOV_CONST_REG, // 常数到寄存器复制
    ADD_REG_REG,  // 寄存器加法
    SUB_REG_REG,  // 寄存器减法
    MUL_REG_REG,  // 寄存器乘法
    DIV_REG_REG,  // 寄存器除法

    // 控制流操作
    JUMP,         // 无条件跳转
    JUMP_IF_ZERO, // 如果零跳转
    HALT          // 停止执行
};

struct Instruction {
    OpCode opCode;
    int operand1;
    int operand2;
    int result;

    Instruction(OpCode code, int op1 = 0, int op2 = 0, int res = 0)
        : opCode(code), operand1(op1), operand2(op2), result(res) {}
};

class RegisterVM {
private:
    std::vector<int> registers;
    std::vector<Instruction> program;
    unsigned int pc; // Program Counter

public:
    RegisterVM(int numRegisters) : registers(numRegisters, 0), pc(0) {}

    int getRegister(int idx){
        return registers[idx];
    }

    void loadProgram(const std::vector<Instruction>& programCode) {
        program = programCode;
    }

    void run() {
        while (pc < program.size()) {
            const Instruction& currentInstruction = program[pc];
            switch (currentInstruction.opCode) {
                case MOV_REG_REG:
                    registers[currentInstruction.result] = registers[currentInstruction.operand1];
                    break;
                case MOV_CONST_REG:
                    registers[currentInstruction.result] = currentInstruction.operand1;
                    break;
                case ADD_REG_REG:
                    registers[currentInstruction.result] = registers[currentInstruction.operand1] + registers[currentInstruction.operand2];
                    break;
                case SUB_REG_REG:
                    registers[currentInstruction.result] = registers[currentInstruction.operand1] - registers[currentInstruction.operand2];
                    break;
                case MUL_REG_REG:
                    registers[currentInstruction.result] = registers[currentInstruction.operand1] * registers[currentInstruction.operand2];
                    break;
                case DIV_REG_REG:
                    if (registers[currentInstruction.operand2] != 0) {
                        registers[currentInstruction.result] = registers[currentInstruction.operand1] / registers[currentInstruction.operand2];
                    } else {
                        std::cerr << "Error: Division by zero." << std::endl;
                        return;
                    }
                    break;
                case JUMP:
                    pc = currentInstruction.operand1;
                    continue;
                case JUMP_IF_ZERO:
                    if (registers[currentInstruction.operand1] == 0) {
                        pc = currentInstruction.operand2;
                        continue;
                    }
                    break;
                case HALT:
                    return;
            }
            pc++;
        }
    }
};

int main() {
    RegisterVM vm(4); // 创建一个具有4个寄存器的虚拟机

    std::vector<Instruction> program = {
        {MOV_CONST_REG, 5, 0, 0},
        {MOV_CONST_REG, 3, 0, 1},
        {ADD_REG_REG, 0, 1, 2},
        {HALT}
    };

    vm.loadProgram(program);
    vm.run();

    std::cout << "Result: " << vm.getRegister(2) << std::endl;

    return 0;
}
