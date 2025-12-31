#include <string.h>
#include <stdio.h>

char* trim(char* szSource)
{
    char* pszEOS = 0;

    //  Set pointer to character before terminating NULL
    pszEOS = szSource + strlen(szSource) - 1;

    //  iterate backwards until non '_' is found
    while ((pszEOS >= szSource) && (*pszEOS == '_'))
        *pszEOS-- = '\0';
    return szSource;
}

int main()
{
    char szbuf[] = "12345_____";

    printf_s("\nBefore trim: %s", szbuf);
    printf_s("\nAfter trim: %s\n", trim(szbuf));
}