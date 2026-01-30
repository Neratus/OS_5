#include <stdio.h>



int division(int dividend, int divider)
{
    int quotient = 0;
    while (dividend >= divider)
    {
        quotient ++;
        dividend -= divider;
    }
    return quotient;
}


int main(void)
{
    int a;
    int d;
    int quotient;
    int dividend;

    printf("Введите целочисленне делимое и делитель : ");
    if (scanf("%d%d", &a, &d) != 2)
    {
        return 1;
    }
 
    if ((a <= 0) || (d <= 0))
    {
        return 1;
    }
    quotient = division(a, d);
    dividend = a - quotient * d;
    printf("Частное от деления равно: %d \n", quotient);
    printf("Остаток от деления равен: %d\n", dividend);
    return 0;
}

