#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "./history.h"

//  hExpand() returns a copy of the string OLDLINE in which any substring of
//  //  the command line of the form !! or !N or !-N or !?STRING? as follows:
//  //
//  //    !!        -> last remembered command
//  //
//  //    !N        -> N-th command read (N is any sequence of digits)
//  //
//  //    !-N       -> N-th last remembered command (N is any sequence of digits)
//  //
//  //    !?STRING? -> most recent remembered command with a token that contains
//  //                 STRING (= a nonempty sequence of characters that does not
//  //                 contain whitespace or !  or ?)
//  //
//  //  The replacement string consists of the specified sequence of tokens
//  //  separated by single blanks.
//  //
//  //  If the notation above is followed by a *, then the leading token in the
//  //  command is omitted.  If it is followed by a ^, then only the first token is
//  //  used.  If it is followed by a :M (where M is any sequence of digits), then
//  //  only the M-th token is used.  If it is followed by a $, then only the last
//  //  token is used.
//  //
//  //  The search for substrings to expand proceeds from left to right and
//  //  continues after each replacement string.  If the requested substitution
//  //  is impossible, then the substring is deleted, but the search continues.
//  //  Storage for the new line is allocated by malloc()/realloc().  The string
//  //  OLDLINE is not modified.
//  //
//  //  hExpand() sets STATUS to 1 if substitutions were made and all succeeded, to
//  //  0 if no substitutions were requested, and to -1 if some substitution failed.
//
//
struct token *tokenHistory[323];
int currentPossition = 0;

char *combine(char *string1, char *string2) {
    int firstLength = strlen(string1);
    int secondLength = strlen(string2);
    char *result = malloc(firstLength + secondLength + 1);
    strcpy(result, string1);
    strcat(result, string2);
    result[firstLength + secondLength] = '\0';
    free(string1);
    free(string2);
    return result;
}

char *stringCopy (char *string) {
    char *copy = malloc (strlen(string) + 1);
    strcpy(copy, string);
    copy[strlen(string)] = '\0';
    return copy;
}

char *combineNoFree(char *string1, char *string2) {
    int firstLength = strlen(string1);
    int secondLength = strlen(string2);
    char *result = malloc(firstLength + secondLength + 1);
    strcpy(result, string1);
    strcat(result, string2);
    result[firstLength + secondLength] = '\0';
    return result;
}

char *convertTokensToString (struct token *list) {
    struct token *current = list;
    char *outputString = malloc(1);
    outputString[0] = '\0';
    while (current) {
        if (outputString[0]) {
            char *lineSpace = malloc(2);
            lineSpace[0] = ' ';
            lineSpace[1] = '\0';
            outputString = combine(outputString, lineSpace);
        }
        char *tokenText = stringCopy(current -> text);
        outputString = combine(outputString, tokenText);
        current = current -> next;
    }
    return outputString;
}

char *getTokenForPos (struct token *list, int pos) {
    int count = 0;
    struct token *current = list;
    while (count != pos) {
        if (current -> next) {
            current = current -> next;
        }
        else {
            return NULL;
        }
        count ++;
    }
    return current -> text;
}

char *getLastToken (struct token *list) {
    struct token *current = list;
    while (current -> next) {
        current = current -> next;
    }
    return current -> text;
}

int getIntLength (int tokenNum) {
    int numLength = 0;
    while (tokenNum >= 1) {
        tokenNum /= 10;
        numLength ++;
    }
    return numLength;
}

char *getTokenSequenceForList (struct token *list,
                               int *iPointer,
                               const char *oldLine,
                               int *replacementStatus) {
    
    if (oldLine[0] == '^'){
        iPointer[0] ++;
        return getTokenForPos (list, 1);
    }
    else if (oldLine[0] == '$'){
        iPointer[0] ++;
        return getLastToken (list);
    }
    else if (oldLine[0] == ':'){
        if (strlen(&oldLine[0]) > 1) {
            if (oldLine[1] >= 48 && oldLine[1] <= 57) {
                char *endptr;
                int tokenNum = strtol (&oldLine[1], &endptr, 10);
                char *tokenReturned = getTokenForPos (list, tokenNum);
                int numLength = getIntLength (tokenNum);
                iPointer[0] += numLength + 1;
                return tokenReturned;
            }
        }
    }
    iPointer[0] --;
    return convertTokensToString(list);
}


char *getReplacement (int *iPointer, const char *oldLine, int *replacementStatus) {
    int lineLength = strlen(oldLine);
    char *endptr;
    struct token *listToSearch;
    int listNum;
    if (oldLine[0] == '!') {
        iPointer[0] += 1;
        if (currentPossition > 0) {
            return getTokenSequenceForList (tokenHistory[currentPossition - 1],
                                            iPointer, &oldLine[iPointer[0]],
                                            replacementStatus);
        }
        else {
            return NULL;
        }
    }
    else if (oldLine[0] == '-'){
        if (lineLength > 1) {
            if (oldLine[1] >= 48 && oldLine[1] <= 57) {
                listNum = strtol (&oldLine[1], &endptr, 10);
                listToSearch = tokenHistory[currentPossition - listNum];
                int numLength = getIntLength (listNum);
                iPointer[0] += numLength + 2;
                return getTokenSequenceForList (listToSearch,
                                                iPointer,
                                                &oldLine[iPointer[0]],
                                                replacementStatus);
            }
        }
    }
    else if (oldLine[0] >= 48 && oldLine[0] <= 57) {
        listNum = strtol (&oldLine[0], &endptr, 10);
        if (listNum > 0 && listNum <= currentPossition) {
            listToSearch = tokenHistory[listNum - 1];
            int numLength = getIntLength (listNum);
            iPointer[0] += numLength + 1;
            return getTokenSequenceForList (listToSearch,
                                            iPointer,
                                            &oldLine[iPointer[0]],
                                            replacementStatus);
        }
    }
    return NULL;
}

char *hExpand (const char *oldLine, int *status) {
    char *expanded = malloc(1);
    expanded[0] = '\0';
    int stringLength = strlen(oldLine);
    int statusTracker = 0;
    int iPointer[1];
    for (int i = 0; i < stringLength - 1; i++) {
        if (oldLine[i] == '!'){
            int replacementStatus[1];
            if (statusTracker != -1){
                statusTracker = 1;
            }
            if (i + 1 < stringLength - 1){
                iPointer[0] = i;
                char *replacement = getReplacement (iPointer, &oldLine[i + 1], replacementStatus);
                if (!replacement) {
                    statusTracker = -1;
                }
                else {
                    expanded = combine (expanded, replacement);
                }
                i = iPointer[0];
            }
        }
        else {
            char *letterToAdd = malloc(2);
            letterToAdd[0] = oldLine[i];
            letterToAdd[1] = '\0';
            expanded = combine (expanded, letterToAdd);
        }
    }
    status[0] = statusTracker;
    
    char *endString = malloc(2);
    endString[0] = '\n';
    endString[1] = '\0';
    expanded = combine(expanded, endString);
    return expanded;
}
//
//
//  // hRemember() adds (a copy of) the command represented by the token list LIST
//  // to the list of remembered commands as the NCMD-th command (where NCMD = 1 on
//  // the first invocation and increases by one on every subsequent invocation).




void hRemember (int ncmd, token *list) {
    currentPossition ++;
    tokenHistory[ncmd - 1] = list;
}

void hClear (void) {

}

// hDump(n) writes the most recent N remembered commands by increasing number
// // to the standard output using something equivalent to
// //
// //   printf ("%6d %s %s ... %s\n", icmd, token0, token1, ..., tokenLast)
// //
// // where ICMD is the number of the command and TOKEN0, ..., TOKENLAST are its
// // tokens.
//
void hDump (int n) {
    int startPos = currentPossition - n;
        while (startPos < currentPossition) {
            printf("%6d  ", startPos);
            struct token *tokenToPrint = tokenHistory[startPos];
            printAllTokens (tokenToPrint);
            printf("\n");
            startPos ++;
        }
}

void printAllTokens (struct token *headToken) {
    struct token *currentToken = headToken;
    while (currentToken -> next) {
        printf("%s ",currentToken -> text);
        currentToken = currentToken -> next;
    }
    printf("%s",currentToken -> text);
}

