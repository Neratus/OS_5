#include <stdio.h>


int main(void)
{
    int sec;
    int min;
    int hour;

    printf("Введите время в секундах: ");
    if (scanf("%d", &sec) != 1)
    {
        return 1;
    }
 
    if (sec <= 0)
    {
        return 1;
    }
  
    min = sec / 60;
    hour = min / 60;
    min = min % 60;
    sec = sec % 60;

    printf("Итоговое время %d : %d : %d\n", hour, min, sec);
    return 0;
}

