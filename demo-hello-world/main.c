//#include <stdio.h>
//
//int main() {
//   printf("Hello, World!\n");
//   return 0;
//}

//// 预处理功能示例
//#include <stdio.h>
//#define SOME_MACRO_VALUE "Hello, MACRO!\n"
//#define SWAN
//
//int main() {
//    printf(SOME_MACRO_VALUE);
//    printf(SWAN);
//#ifdef SWAN
//    printf("Hello, Swan!\n");
//#endif
//    return 0;
//}

//// 预处理语法
//#include <stdio.h>
//int main() {
//    2();
//}

//// 预处理词法
//    #include <stdio.h>
//int main() {
//    //#include <stdio.h>
//    @#include <stdio.h>!@#$%^&*())(*&^%$#@!~
//}

// 外部符号
#include <stdio.h>

extern int a(void);
int main() {
   printf("Hello, %d!\n", a());
   return 0;
}

