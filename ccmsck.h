#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

/////////////型の宣言/////////////
//トークンの種類
typedef enum{
  TK_RESERVED,//記号
  TK_IDENT,   //識別子
  TK_NUM,     //整数トークン
  TK_EOF      //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;
struct Token
{
  TokenKind kind; //トークンの型
  Token *next;  //次の入力トークン
  int val;     //kindがTK_NUMの場合，その数値
  char *str;   //トークン文字列
  int len;     //トークンの長さ
};

typedef enum{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_ASSIGN, // =
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_LVAR, // ローカル変数
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;
//抽象構文木のノードの型
struct Node {
  NodeKind kind;
  Node *lhs;  //右辺
  Node *rhs;  //左辺
  int val;    //kindがND_NUMの場合のみ使う
  int offset; //kindがND_LVARの場合のみ使う
};

/////////////プロトタイプ宣言/////////////
//main.c
//container.c
void error_at(char *loc,char *fmt, ... );
void error(char *fmt, ... );
//codegen.c
void gen(Node *node);
//parse.c
bool expect(char *op);
int expect_num();
bool consume(char *op);
bool at_eof();
bool startswith(char *p,char *q);
Token *new_token(TokenKind kind,Token *cur,char *str,int len);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs,Node *rhs);
Node *new_node_num(int val);
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void program();

/////////////グローバル変数の宣言/////////////
//main.c
extern char *user_input;
//container.c
//codegen.c
//parse.c
extern Token *token;
extern Node *code[100]; //;区切りごとにNode列を格納する．(一文ずつ)