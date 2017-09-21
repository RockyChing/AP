#include <stdio.h>


#include <types.h>
#include <log.h>



static void check_builtin_type_size()
{
    LOG("sizeof(char): %d", sizeof(char));
    LOG("sizeof(short): %d", sizeof(short));
    LOG("sizeof(long): %d", sizeof(long));
    LOG("sizeof(long long): %d", sizeof(long long));
}













int main(int argc, char **argv)
{
    (void)argc;
    printf("%s enter.\n", argv[0]);
    check_builtin_type_size();





    printf("%s exit.\n", argv[0]);
    return 0;
}

