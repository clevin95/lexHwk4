#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c/cs323/Hwk4/history.h"


//GlobalVariables
struct token **tokenHistory = NULL;
int currentPossition = 0;   //Next possition in history to be filled
int historyTotal = 0;       //Total # of items added
int currentCount = 0;       //Number of items since last clear
int historySize = 323;


//Creates a new string equal to string1 + string2
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


//Returns the correct index for the possition in history
//Returns 1 if it has been cleared or overwritten
int getIndexHistoryPossition (int input){
    int relativePosition = currentCount - (historyTotal - input);
    if (input > historyTotal ||
        input <= historyTotal - currentCount ||
        (relativePosition <= currentCount - historySize && historyTotal > historySize)) {
        return -1;
    }
    return (relativePosition - 1) % historySize;
}

char *stringCopy (char *string) {
    char *copy = malloc (strlen(string) + 1);
    strcpy(copy, string);
    copy[strlen(string)] = '\0';
    return copy;
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

//Creates a string out of all the texts in a token list
//Excluding the first token
char *convertAllButFirst (struct token *list) {
    struct token *current = list;
    char *outputString = malloc(1);
    outputString[0] = '\0';
    while (current -> next) {
        if (outputString[0]) {
            char *lineSpace = malloc(2);
            lineSpace[0] = ' ';
            lineSpace[1] = '\0';
            outputString = combine(outputString, lineSpace);
        }
        char *tokenText = stringCopy(current -> next -> text);
        outputString = combine(outputString, tokenText);
        current = current -> next;
    }
    return outputString;
}

//Returns the text for the token at possition pos
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
    return stringCopy(current -> text);
}

//Gets the last token of a list
char *getLastToken (struct token *list) {
    struct token *current = list;
    while (current -> next) {
        current = current -> next;
    }
    return stringCopy(current -> text);
}

//Gets the length of an integer
int getIntLength (int tokenNum) {
    int numLength = 0;
    while (tokenNum >= 1) {
        tokenNum /= 10;
        numLength ++;
    }
    return numLength;
}


//Determines which token(s) from the list that should be in the replacement
char *getTokenSequenceForList (struct token *list,
                               int *iPointer,
                               const char *oldLine,
                               int *replacementStatus) {
    if (oldLine[iPointer[0]] == '^'){
        return getTokenForPos (list, 1);
    }
    else if (oldLine[iPointer[0]] == '$'){
        return getLastToken (list);
    }
    else if (oldLine[iPointer[0]] == ':'){
        if (strlen(&oldLine[iPointer[0]]) > 1) {
            if (oldLine[iPointer[0] + 1] >= 48 && oldLine[iPointer[0] + 1] <= 57) {
                char *endptr;
                int tokenNum = strtol (&oldLine[iPointer[0] + 1], &endptr, 10);
                char *tokenReturned = getTokenForPos (list, tokenNum);
                int numLength = getIntLength (tokenNum);
                iPointer[0] += numLength;
                return tokenReturned;
            }
        }
    }
    else if (oldLine[iPointer[0]] == '*'){
        return convertAllButFirst (list);
    }
    iPointer[0] --;
    return convertTokensToString(list);
}


//Extracts the string from !?STRING?
char *getString (const char *oldLine) {
    int length = strlen(oldLine);
    char *outputString = malloc(length);
    int i;
    for (i = 0; oldLine[i] != '?' ; i ++) {
        if (!oldLine[i] || oldLine[i] == '\n' || oldLine[i] == '!') {
            break;
        }

        outputString[i] = oldLine[i];
    }
    outputString[i] = '\0';
    return outputString;
}

int searchTokenList (char *string, struct token *list) {
    struct token *current = list;
    while (current) {
        if (strstr(current -> text, string)) {
            return 1;
        }
        current = current -> next;
    }
    return 0;
}

struct token *findStringInHistory (char *string) {
    for (int i = currentPossition - 1; i >= 0; i --) {
        if (searchTokenList(string, tokenHistory[i])){
            return tokenHistory[i];
        }
    }
    return NULL;
}


//Skips to the end of an escape sequence
void skipToEnd (const char *oldLine, int *iPointer) {
    char *characters = "^*$";
    int foundChar = 0;
    for (int i = 0; i < 4; i++) {
        if(oldLine[iPointer[0] + 1] == characters[i]) {
            foundChar = 1;
        }
    }
    if (foundChar){
        iPointer[0] ++;
    }
    else if (oldLine[iPointer[0] + 1] == ':' ){
        if (strlen(&oldLine[iPointer[0]]) > 1) {
            if (oldLine[iPointer[0] + 2] >= 48 && oldLine[iPointer[0] + 2] <= 57) {
                char *endptr;
                int tokenNum = strtol (&oldLine[iPointer[0] + 2], &endptr, 10);
                int numLength = getIntLength (tokenNum);
                iPointer[0] += numLength + 1;
                return;
            }
        }
    }
    else {
        return;
    }
}

//Determines if a replacement designator has been passed in and
//replases and locates the appropriate list
char *getReplacement (int *iPointer, const char *oldLine, int *replacementStatus) {
    int lineLength = strlen(&oldLine[iPointer[0]]);
    char *endptr;
    struct token *listToSearch;
    int listNum;
    if (oldLine[iPointer[0]] == '!') {
        if (!tokenHistory){
            if (oldLine[iPointer[0] + 1]) {
                skipToEnd(oldLine, iPointer);
            }
            return NULL;
        }
        iPointer[0] += 1;
        return getTokenSequenceForList (tokenHistory[currentPossition - 1],
                                        iPointer, oldLine,
                                        replacementStatus);

        replacementStatus[0] = 1;
    }
    else if (oldLine[iPointer[0]] == '-'){
        if (lineLength > 1) {
            if (oldLine[iPointer[0] + 1] >= 48 && oldLine[iPointer[0] + 1] <= 57) {
                listNum = strtol (&oldLine[iPointer[0] + 1], &endptr, 10);
                int index = getIndexHistoryPossition (historyTotal + 1 - listNum);
                int numLength = getIntLength (listNum);
                iPointer[0] += numLength + 1;
                if (!tokenHistory){
                    if (oldLine[iPointer[0] + 1]) {
                        skipToEnd(oldLine, iPointer);
                    }
                    return NULL;
                }
                if (index >= 0){
                    listToSearch = tokenHistory[index];
                    return getTokenSequenceForList (listToSearch,
                                                    iPointer,
                                                    oldLine,
                                                    replacementStatus);
                }
            }
        }
        replacementStatus[0] = 1;
    }
    else if (oldLine[iPointer[0]] >= 48 && oldLine[iPointer[0]] <= 57) {
        listNum = strtol (&oldLine[iPointer[0]], &endptr, 10);
        int index = getIndexHistoryPossition(listNum);
        int numLength = getIntLength (listNum);
        iPointer[0] += numLength;
        if (!tokenHistory){
            if (oldLine[iPointer[0] + 1]) {
                if (oldLine[iPointer[0] + 1] ==':') {
                    iPointer[0] ++;
                }
                skipToEnd(oldLine, iPointer);
            }
            return NULL;
        }
        if (index >= 0){
            listToSearch = tokenHistory[index];
            return getTokenSequenceForList (listToSearch,
                                            iPointer,
                                            oldLine,
                                            replacementStatus);
        }
        replacementStatus[0] = 1;
    }
    else if (oldLine[iPointer[0]] == '?') {
        if (lineLength > 1) {
            char *searchString = getString (&oldLine[iPointer[0] + 1]);
            listToSearch = findStringInHistory (searchString);
            iPointer[0] += strlen(searchString) + 1;
            if (oldLine[iPointer[0]] == '?'){
                iPointer[0] ++;
            }
            if (!tokenHistory || !listToSearch){
                if (oldLine[iPointer[0] + 1]) {
                    skipToEnd(oldLine, iPointer);
                }
                return NULL;
            }
            free(searchString);
            return getTokenSequenceForList (listToSearch,
                                            iPointer,
                                            oldLine,
                                            replacementStatus);
        }
        replacementStatus[0] = 1;
    }
    if (replacementStatus[0] != 1) {
        replacementStatus[0] = -1;
    }
    return NULL;
}

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
char *hExpand (const char *oldLine, int *status) {
    char *expanded = malloc(1);
    expanded[0] = '\0';
    int stringLength = strlen(oldLine);
    int statusTracker = 0;
    int iPointer[1];
    for (int i = 0; i < stringLength - 1; i++) {
        if (oldLine[i] == '!' && i + 1 < stringLength - 1){
            int replacementStatus[1];
            iPointer[0] = i;
            replacementStatus[0] = 0;
            iPointer[0] ++;
            char *replacement = getReplacement (iPointer, oldLine, replacementStatus);
            if (replacementStatus[0] == -1) {
                char *letterToAdd = malloc(2);
                letterToAdd[0] = oldLine[i];
                letterToAdd[1] = '\0';
                expanded = combine (expanded, letterToAdd);
            }
            else if (!replacement) {
                statusTracker = -1;
                i = iPointer[0];
            }
            else if (replacementStatus[0] != -1) {
                if (statusTracker != -1){
                    statusTracker = 1;
                }
                expanded = combine (expanded, replacement);
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


//Creates the array where tokens are storred
void createTokenHistory (void) {
    char *histString = getenv("HISTSIZE");
    if (histString) {
        historySize = atoi(histString);
    }
    if (!tokenHistory) {
        tokenHistory = malloc(historySize * sizeof(struct token *));
        for (int i = 0; i < historySize; i ++) {
            tokenHistory[i] = NULL;
        }
    }
}

//  // hRemember() adds (a copy of) the command represented by the token list LIST
//  // to the list of remembered commands as the NCMD-th command (where NCMD = 1 on
//  // the first invocation and increases by one on every subsequent invocation).
void hRemember (int ncmd, token *list) {
    createTokenHistory();
    char *tokenString = convertTokensToString(list);
    if (tokenHistory[currentPossition]) {
        freeList(tokenHistory[currentPossition]);
    }
    tokenHistory[currentPossition] = lex(tokenString);
    free(tokenString);
    currentCount ++;
    historyTotal = ncmd;
    currentPossition = currentCount % historySize;
}

//Clears all the tokens from history
void hClear (void) {
    if (tokenHistory){
        for (int i = 0; i < historySize; i++) {
            if (tokenHistory[i]){
                freeList(tokenHistory[i]);
                tokenHistory[i] = NULL;
            }
        }
        free(tokenHistory);
        tokenHistory = NULL;
    }
    currentPossition = 0;
    currentCount = 0;
}

//Prints all tokens in a list
void printAllTokens (struct token *headToken) {
    struct token *currentToken = headToken;
    while (currentToken -> next) {
        printf("%s ",currentToken -> text);
        currentToken = currentToken -> next;
    }
    printf("%s\n",currentToken -> text);
}

// hDump(n) writes the most recent N remembered commands by increasing number
// // to the standard output using something equivalent to
// //
// //   printf ("%6d %s %s ... %s\n", icmd, token0, token1, ..., tokenLast)
// //
// // where ICMD is the number of the command and TOKEN0, ..., TOKENLAST are its
// // tokens.

void hDump (int n) {
    int start = historyTotal - (n - 1);
    while (start <= historyTotal) {
        int index = getIndexHistoryPossition(start);
        if (index >= 0) {
            printf("%6d  ", start);
            struct token *tokenToPrint = tokenHistory[index];
            printAllTokens (tokenToPrint);
        }
        start ++;
    }
}
