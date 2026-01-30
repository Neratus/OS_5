/*
 * filename: calculator.x
 * process in remote calls
 */

const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct CALCULATOR_ARGS {
    int op;
    float arg1;
    float arg2;
};

program CALCULATOR_PROG {
    version CALCULATOR_VER {
        float CALCULATE(CALCULATOR_ARGS) = 1;
    } = 1;
} = 0x20000001;
