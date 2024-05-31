#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { SEMI, OPEN_PAREN, CLOSE_PAREN, EXIT, INT } Types;

typedef struct {
  Types type;
  int value;
} Token;

typedef struct TokenList {
  Token token;
  struct TokenList *next;
} TokenList;

TokenList *tokens = NULL;

TokenList *allocate_list_node(Token token) {
  TokenList *node = (TokenList *)malloc(sizeof(TokenList));

  node->token = token;
  node->next = NULL;

  return node;
}

TokenList *add_to_tail(TokenList *head, Token token) {
  TokenList *node = allocate_list_node(token);

  if (head == NULL)
    return node;

  TokenList *p = head;

  while (p->next != NULL) {
    p = p->next;
  }

  p->next = node;
  return head;
}

Token lex_number(char current, FILE *file) {

  Token token;
  token.type = INT;
  char *value = malloc(sizeof(char) * 8);
  int counter = 0;

  int int_value = 0;

  while (isdigit(current) && current != EOF) {
    if (counter >= 8) {
      printf("Integer overflow over 8 digits\n");
    }
    value[counter] = current;
    counter++;
    current = fgetc(file);
  }

  for (int i = counter - 1; i >= 0; i--) {
    int_value += ((int)value[i] - '0') * pow(10, counter - i - 1);
  }

  token.value = int_value;
  fseek(file, -1, SEEK_CUR);

  free(value);

  return token;
}

Token lex_word(char current, FILE *file) {

  Token token;
  char *value = malloc(sizeof(char) * 1024);
  int counter = 0;

  while (isalpha(current) && current != EOF) {
    if (counter >= 1023) {
      printf("Keyword overflow over 1023 letters\n");
    }
    value[counter] = current;
    counter++;
    current = fgetc(file);
  }

  value[counter] = '\0';

  if (strcmp(value, "exit") == 0) {
    token.type = EXIT;
  } else {
    printf("Syntax error: keyword not recognized: %s\n", value);
    free(value);
    exit(1);
  }

  fseek(file, -1, SEEK_CUR);

  free(value);

  return token;
}

void lexer(FILE *file) {
  char current = fgetc(file);
  Token token;

  while (current != EOF) {
    if (current == ';') {

      token.type = SEMI;
      token.value = 0;
      tokens = add_to_tail(tokens, token);

    } else if (current == '(') {

      token.type = OPEN_PAREN;
      token.value = 0;
      tokens = add_to_tail(tokens, token);

    } else if (current == ')') {

      token.type = CLOSE_PAREN;
      token.value = 0;
      tokens = add_to_tail(tokens, token);

    } else if (isdigit(current)) {

      token = lex_number(current, file);
      tokens = add_to_tail(tokens, token);

    } else if (isalpha(current)) {

      token = lex_word(current, file);
      if (token.type == EXIT) {
        tokens = add_to_tail(tokens, token);
      }
    }

    current = fgetc(file);
  }
}

void free_tokens() {
  TokenList *p = tokens;
  TokenList *temp = NULL;

  while (p != NULL) {
    temp = p->next;
    free(p);
    p = temp;
  }
}

void codegen(int code) {
  FILE *out;
  out = fopen("out.s", "w");

  fprintf(out, ".global _start\n_start:\n  mov    X0, #");
  fprintf(out, "%d", code);
  fprintf(out, "\n  mov    X16, #1\n  svc    #0x80");

  fclose(out);
}

void parse() {
  TokenList *p = tokens;
  int code = 0;
  while (p != NULL) {

    if (p->token.type == EXIT) {
      if (p->next)
        p = p->next;
      if (p->token.type == OPEN_PAREN) {
        if (p->next)
          p = p->next;
        if (p->token.type == INT) {
          code = p->token.value;
          if (p->next)
            p = p->next;
          if (p->token.type == CLOSE_PAREN) {
            if (p->next)
              p = p->next;
            if (p->token.type == SEMI) {
              codegen(code);
              system("make");
            }
          }
        }
      }
    }

    p = p->next;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Correct usage: ./main <filename>");
    exit(1);
  }

  FILE *file;
  if (access(argv[1], F_OK) == 0) {

    file = fopen(argv[1], "r");
    lexer(file);
    parse();
    free_tokens();
  } else {
    printf("File %s not found", argv[1]);
    exit(1);
  }

  return 0;
}
