#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークン
typedef enum
{
    TK_RESERVED, //記号
    TK_NUM,      //整数トークン
    TK_EOF,      //終端文字
} TokenKind;

typedef struct Token Token;
struct Token
{
    TokenKind kind; //トークンの型
    Token *next;    //次のトークンへのポインタ
    int value;      //TK_NUMの場合の数値
    char *str;      //入力文字列から生成されたトークン文字列
    int len;        //トークンの長さ
};

//ノード
typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;
struct Node
{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺に来るノード
    Node *rhs;     //右辺に来るノード
    int value;     //kindがND_NUMの場合の数値
};


//プロトタイプ宣言
//parse.c
Token *tokenize(char *p);
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
//codegen.c
void gen(Node *node);
//container.c
void error_at(char *loc, char *fmt, ...);

//グローバル変数
extern Token *token;
extern char *user_input;