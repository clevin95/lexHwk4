#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "./lex.h"

int letterIsRepeatableNonSimple (char letter) {
    if (letter == '>') {
        return 1;
    }
    else if (letter == '<') {
        return 1;
    }
    else if (letter == '&'){
        return 1;
    }
    else if (letter == '|'){
        return 1;
    }
    return 0;
}

int letterIsNonSimple (char letter) {
    char * nonSimple = "<>;&|()";
    for (int i = 0; i < 7; i ++) {
        if (nonSimple[i] == letter) {
            return 1;
        }
    }
    return 0;
}


int getType (char *string)    {
    if (!strcmp(string, "<")) {
        return REDIR_IN;
    }
    else if (!strcmp(string, "<<")) {
        return REDIR_HERE;
    }
    else if (!strcmp(string, ">")) {
        return REDIR_OUT;
    }
    else if (!strcmp(string, ">>")) {
        return REDIR_APP;
    }
    else if (!strcmp(string, "&")) {
        return SEP_BG;
    }
    else if (!strcmp(string, "&&")) {
        return SEP_AND;
    }
    else if (!strcmp(string, "|")) {
        return REDIR_PIPE;
    }
    else if (!strcmp(string, "||")) {
        return SEP_OR;
    }
    else if (!strcmp(string, ")")) {
        return PAREN_RIGHT;
    }
    else if (!strcmp(string, "(")) {
        return PAREN_LEFT;
    }
    else if (!strcmp(string, ";")) {
        return SEP_END;
    }
    return SIMPLE;
}

int getTokenLength (const char *line, int possition) {
    int lineLength = strlen(line);
    int i = possition;
    for (i = possition; i < lineLength; i ++) {
        if (letterIsNonSimple(line[i])) {
            if (i == possition){
                if (letterIsRepeatableNonSimple(line[i])) {
                    if (i + 1 < lineLength) {
                        if (line[i] == line[i + 1]){
                            i ++;
                        }
                    }
                }
                i++;
            }
            break;
        }
        
        if (!isgraph(line[i])) {
            break;
        }
    }
    return i - possition;
}


char *removeLineBreak (char *line) {
    int lineLength = strlen(line);
    if (line[lineLength - 1] == '\n') {
        line = realloc(line, lineLength);
        line[lineLength - 1] = '\0';
    }
    
    return line;
}


int getNextToken (const char *line, int possition, struct token **newToken) {
    int length = getTokenLength(line, possition);
    char *tokenText = malloc(length + 1);
    tokenText[length] = '\0';
    int i;
    for (i = 0; i < length; i ++) {
        tokenText[i] = line[i + possition];
    }
    (*newToken) -> text = removeLineBreak(tokenText);
    (*newToken) -> type = getType(tokenText);
    return possition + length - 1;
}


token *lex (const char *line) {
    if (!line) {
        return NULL;
    }
    
    int lineLength = strlen(line);
    struct token *nextToken;
    struct token *lastToken  = NULL;
    struct token *headPointer  = NULL;
    for (int i = 0; i < lineLength; i++) {
        if (isgraph(line[i])) {
            nextToken = malloc (sizeof(struct token));
            i = getNextToken (line, i, &nextToken);
            nextToken -> next = NULL;
            if (lastToken) {
                lastToken -> next = nextToken;
            }
            else {
                headPointer = nextToken;
            }
            lastToken = nextToken;
        }
    }
    return headPointer;
}

//void printAllTokens (struct token *headToken) {
//    struct token *currentToken = headToken;
//    while (currentToken -> next) {
//        printf("%s ",currentToken -> text);
//        currentToken = currentToken -> next;
//    }
//    printf("%s",currentToken -> text);
//}



//void freeList (struct token *list) {
//    struct token *current = list;
//    struct token *nextHolder;
//    while (current)  {
//        nextHolder = current->next;
//        free(current->text);
//        free(current);
//        current = nextHolder;
//    }
//}



//int main(int argc, char **argv){
//    struct token *testToken;
//    testToken = lex (" this is a sentance");
//    printAllTokens (testToken);
//    return 1;
//}

