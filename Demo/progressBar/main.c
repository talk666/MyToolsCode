#include <stdio.h>
#include <string.h>
#include <unistd.h>


// ****************************** 配置 ***************************
// 最后100%时的输出形式  
const char * LastStr = "[--------------------] 100%"; 

// 进度条标志，可以改用"*"或其它符号
const char ProgressIcon = '-'; 

// 进度每加5，进度条就会加一格，注意Step值要和LastStr中的"-"数量相匹配，两者相乘等于100
const int Step = 5; 

// 总共需要多少进度标志，即LastStr中"-"的数量
const int IconMaxNum = 20; 

// 每隔100ms打印一次，这里同时每隔100ms会让进度加1
const int PrintInterval = 100000; 
// ****************************************************************


// main函数
int main(void)
{
    printf("\nOutput recorded data: ");
    int i,j;    
    for (i = 0; i <= 100; ++i)
    {
        // -------------- 打印进度条 --------------
        printf("[");
        int currentIndex = i / Step;
        for (j = 0; j < IconMaxNum; ++j)
        {
            if (j < currentIndex)
            {
                printf("%c", ProgressIcon); // 打印进度条标志
            }
            else
            {
                printf(" "); // 未达进度则打印空格
            }
        }
        
        printf("] ");
        // ----------------------------------------

        // -------- 打印数字进度 ----------
        printf("%3d%%", i);
        fflush(stdout);
        // -------------------------------


        usleep(PrintInterval);
        
        for (j = 0; j < strlen(LastStr); ++j)
        {
            printf("\b"); // 回删字符，让数字和进度条原地变化
        }
        
        fflush(stdout);
    }
    
    printf("\n\n");
    
    return 0;
}
