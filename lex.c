#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIMPLE          10      // Maximal contiguous sequence of nonblanks

#define REDIR_IN        20      // <
#define REDIR_HERE      21      // <<

#define REDIR_PIPE      30      // |
#define REDIR_OUT       31      // >
#define REDIR_APP       32      // >>

#define SEP_END         40      // ;
#define SEP_BG          41      // &
#define SEP_AND         42      // &&
#define SEP_OR          43      // ||

#define PAREN_LEFT      50      // (
#define PAREN_RIGHT     51      // )

#define METACHARS       "<>;&|()"       // Chars that start a non-SIMPLE

typedef struct token {          // Struct for each token in linked list
    char *text;                   //   String containing token
    int type;                     //   Corresponding type
    struct token *next;           //   Pointer to next token in linked list
} token;


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
             printf("Start\n");
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
        
        if (line[i] == ' ') {
            break;
        }
    }
    return i - possition;
}

int getNextToken (const char *line, int possition, struct token **newToken) {
    int length = getTokenLength(line, possition);
    char *tokenText = malloc(length + 1);
    tokenText[length] = '\0';
    int i;
    for (i = 0; i < length; i ++) {
        tokenText[i] = line[i + possition];
    }
    (*newToken) -> text = tokenText;
    (*newToken) -> type = getType(tokenText);
    return possition + length - 1;
}

token *lex (const char *line) {
    int lineLength = strlen(line);
    struct token *nextToken;
    struct token *lastToken  = NULL;
    struct token *headPointer  = NULL;
    for (int i = 0; i < lineLength; i++) {
        if (line[i] != ' ') {
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

void printAllTokens (struct token *headToken) {
    struct token *currentToken = headToken;
    while (currentToken -> next) {
        printf("Token Text : %s   Code : %d\n",currentToken -> text, currentToken -> type);
        currentToken = currentToken -> next;
    }
    printf("Token Text : %s   Code : %d\n",currentToken -> text, currentToken -> type);
}

void freeList (token *list) {
    struct token *currentToken = list;
    while (currentToken -> next) {
        free(currentToken -> text);
        struct token *tokenHolder = currentToken;
        currentToken = currentToken -> next;
        free (tokenHolder);
    }
    free(currentToken);
}


