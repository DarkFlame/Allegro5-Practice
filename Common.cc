#include "Common.h"

void ResetString(char *&sString, const char *pNewString)
{
    //--Courtesy of Salty Justice
    //--Deallocates a string and sets it to the pNewString.  Passing NULL for the pNewString will
    //  simple deallocate and NULL the sString.
    //--MAKE SURE the sString was either NULL or a valid string before passing in to this, this call
    //  should not be used on raw allocated memory.
    if(sString == pNewString) return;

    free(sString);
    sString = NULL;
    if(!pNewString) return;

    sString = (char *)malloc(sizeof(char) * (strlen(pNewString) + 1));
    strcpy(sString, pNewString);
}
